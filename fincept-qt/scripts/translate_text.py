""".
Translation service — multi-API parallel translation.
APIs: MyMemory (primary free), Baidu (secondary), Tencent (tertiary), Google (fallback).
Round-robin across available APIs to maximize throughput.

Commands:
  single <text> [source_lang] [target_lang] [preferred_api]
  article <headline> [summary] [source_lang] [target_lang] [preferred_api]
  detect <text>
  batch <json_array> [source_lang] [target_lang]
"""
import sys
import json
import hashlib
import random
import os
import time
import hmac
import base64
import uuid
import urllib.request
import urllib.parse

_google_available = False
try:
    from deep_translator import GoogleTranslator
    _google_available = True
except ImportError:
    pass

_round_robin_idx = 0
_mymemory_daily_count = 0
_mymemory_daily_limit = 5000
_mymemory_error_streak = 0


def detect_language(text):
    if not text:
        return "en"
    cjk = sum(1 for c in text if '\u4e00' <= c <= '\u9fff')
    jp = sum(1 for c in text if '\u3040' <= c <= '\u30ff')
    kr = sum(1 for c in text if '\uac00' <= c <= '\ud7af')
    total = len(text)
    if total == 0:
        return "en"
    if cjk / total > 0.1:
        return "zh"
    if jp / total > 0.1:
        return "ja"
    if kr / total > 0.1:
        return "ko"
    return "en"


def fix_translation(text):
    fixes = {
        "近平西": "近平",
        "主席西": "主席习",
        "总书记西": "总书记习",
        "西近平": "习近平",
    }
    for wrong, right in fixes.items():
        text = text.replace(wrong, right)
    return text


def _baidu_credentials():
    appid = os.environ.get("BAIDU_TRANSLATE_APPID", "")
    key = os.environ.get("BAIDU_TRANSLATE_KEY", "")
    return appid, key


def _tencent_credentials():
    sid = os.environ.get("TENCENT_SECRET_ID", "")
    skey = os.environ.get("TENCENT_SECRET_KEY", "")
    return sid, skey


def translate_mymemory(text, source="auto", target="zh"):
    """MyMemory Translation API — free tier with daily limit."""
    global _mymemory_daily_count, _mymemory_error_streak

    if _mymemory_daily_count >= _mymemory_daily_limit:
        return None, "MyMemory daily limit reached"
    if _mymemory_error_streak >= 5:
        return None, "MyMemory error streak too high"

    try:
        src_lang = "en" if source in ("auto", "en") else source
        tgt_lang = "zh-CN" if target == "zh" else target

        params = urllib.parse.urlencode({
            'q': text,
            'langpair': f"{src_lang}|{tgt_lang}",
        })
        url = f"https://api.mymemory.translated.net/get?{params}"

        req = urllib.request.Request(url, headers={'User-Agent': 'FinceptTerminal/1.0'})
        with urllib.request.urlopen(req, timeout=10) as resp:
            data = json.loads(resp.read().decode('utf-8'))

        _mymemory_daily_count += 1

        if data.get("responseStatus") == 200:
            translated = data.get("responseData", {}).get("translatedText", "")
            if translated and translated.upper().startswith("MYMEMORY WARNING"):
                _mymemory_error_streak += 1
                _mymemory_daily_count = _mymemory_daily_limit
                return None, "MyMemory daily quota exceeded"
            if translated and translated.lower() != text.lower():
                _mymemory_error_streak = 0
                return translated, None
            else:
                return None, "MyMemory returned empty or same text"
        else:
            _mymemory_error_streak += 1
            return None, f"MyMemory error: {data.get('responseDetails', 'unknown')}"

    except Exception as e:
        _mymemory_error_streak += 1
        return None, f"MyMemory exception: {str(e)}"


def available_apis():
    apis = ["mymemory"]
    appid, key = _baidu_credentials()
    if appid and key:
        apis.append("baidu")
    sid, skey = _tencent_credentials()
    if sid and skey:
        apis.append("tencent")
    if _google_available:
        apis.append("google")
    return apis


def next_api():
    global _round_robin_idx
    apis = available_apis()
    if not apis:
        return None
    api = apis[_round_robin_idx % len(apis)]
    _round_robin_idx += 1
    return api


def translate_baidu(text, appid, key, source="auto", target="zh"):
    try:
        import http.client
        import urllib.parse
        salt = str(random.randint(32768, 65536))
        sign_str = appid + text + salt + key
        sign = hashlib.md5(sign_str.encode('utf-8')).hexdigest()
        params = urllib.parse.urlencode({
            'q': text,
            'from': source,
            'to': target,
            'appid': appid,
            'salt': salt,
            'sign': sign,
        })
        conn = http.client.HTTPSConnection("fanyi-api.baidu.com")
        conn.request("GET", "/api/trans/vip/translate?" + params)
        resp = conn.getresponse()
        data = json.loads(resp.read().decode('utf-8'))
        conn.close()
        if 'error_code' in data:
            return None, f"Baidu error {data['error_code']}: {data.get('error_msg', '')}"
        result = ''.join(item['dst'] for item in data.get('trans_result', []))
        return result, None
    except Exception as e:
        return None, str(e)


def translate_tencent(text, secret_id, secret_key, source="auto", target="zh"):
    try:
        import http.client
        service = "tmt"
        host = "tmt.tencentcloudapi.com"
        endpoint = "https://tmt.tencentcloudapi.com"
        action = "TextTranslate"
        version = "2018-03-21"
        region = "ap-beijing"

        src_lang = "auto" if source == "auto" else source
        tgt_lang = target
        if tgt_lang == "zh":
            tgt_lang = "zh"
        if src_lang == "zh":
            src_lang = "zh"

        payload = json.dumps({
            "SourceText": text,
            "Source": src_lang,
            "Target": tgt_lang,
            "ProjectId": 0,
        })

        timestamp = int(time.time())
        date = time.strftime("%Y-%m-%d", time.gmtime(timestamp))

        credential_scope = f"{date}/{service}/tc3_request"
        signed_headers = "content-type;host;x-tc-action"
        http_method = "POST"
        canonical_uri = "/"
        canonical_querystring = ""
        ct = "application/json; charset=utf-8"
        canonical_headers = f"content-type:{ct}\nhost:{host}\nx-tc-action:{action.lower()}\n"
        hashed_payload = hashlib.sha256(payload.encode("utf-8")).hexdigest()
        canonical_request = f"{http_method}\n{canonical_uri}\n{canonical_querystring}\n{canonical_headers}\n{signed_headers}\n{hashed_payload}"

        algorithm = "TC3-HMAC-SHA256"
        hashed_canonical = hashlib.sha256(canonical_request.encode("utf-8")).hexdigest()
        string_to_sign = f"{algorithm}\n{timestamp}\n{credential_scope}\n{hashed_canonical}"

        def _hmac_sha256(key, msg):
            return hmac.new(key, msg.encode("utf-8"), hashlib.sha256).digest()

        secret_date = _hmac_sha256(("TC3" + secret_key).encode("utf-8"), date)
        secret_service = _hmac_sha256(secret_date, service)
        secret_signing = _hmac_sha256(secret_service, "tc3_request")
        signature = hmac.new(secret_signing, string_to_sign.encode("utf-8"), hashlib.sha256).hexdigest()

        authorization = f"{algorithm} Credential={secret_id}/{credential_scope}, SignedHeaders={signed_headers}, Signature={signature}"

        headers = {
            "Authorization": authorization,
            "Content-Type": ct,
            "Host": host,
            "X-TC-Action": action,
            "X-TC-Timestamp": str(timestamp),
            "X-TC-Version": version,
            "X-TC-Region": region,
        }

        conn = http.client.HTTPSConnection(host)
        conn.request("POST", "/", body=payload, headers=headers)
        resp = conn.getresponse()
        data = json.loads(resp.read().decode('utf-8'))
        conn.close()

        resp_data = data.get("Response", {})
        if "Error" in resp_data:
            return None, f"Tencent error {resp_data['Error'].get('Code', '')}: {resp_data['Error'].get('Message', '')}"

        target_text = resp_data.get("TargetText", "")
        if not target_text:
            return None, "Tencent: empty result"
        return target_text, None
    except Exception as e:
        return None, str(e)


def translate_google(text, source="auto", target="zh"):
    if not _google_available:
        return None, "Google translator not available"
    try:
        src = source if source != "auto" else "auto"
        translated = GoogleTranslator(source=src, target=target).translate(text)
        return translated, None
    except Exception as e:
        return None, str(e)


def translate_with_api(text, api, source="auto", target="zh"):
    if api == "mymemory":
        return translate_mymemory(text, source, target)
    elif api == "baidu":
        appid, key = _baidu_credentials()
        if appid and key:
            return translate_baidu(text, appid, key, source, target)
        return None, "Baidu credentials not set"
    elif api == "tencent":
        sid, skey = _tencent_credentials()
        if sid and skey:
            return translate_tencent(text, sid, skey, source, target)
        return None, "Tencent credentials not set"
    elif api == "google":
        return translate_google(text, source, target)
    return None, f"Unknown API: {api}"


def translate_single(text, source="auto", target="zh", preferred_api=None):
    if not text or not text.strip():
        return {"original": text, "translated": text, "detected_lang": "en", "translator": "none"}

    detected = detect_language(text)
    if detected == target and source in ("auto", target):
        return {"original": text, "translated": text, "detected_lang": detected, "translator": "skip"}

    apis = available_apis()
    if not apis:
        return {"original": text, "translated": text, "detected_lang": detected,
                "translator": "failed", "error": "No translation API available"}

    if preferred_api and preferred_api in apis:
        order = [preferred_api] + [a for a in apis if a != preferred_api]
    else:
        order = apis[:]

    errors = []
    for api in order:
        result, err = translate_with_api(text, api, source, target)
        if result is not None:
            result = fix_translation(result)
            return {"original": text, "translated": result, "detected_lang": detected, "translator": api}
        errors.append(f"{api}: {err}")

    return {"original": text, "translated": text, "detected_lang": detected,
            "translator": "failed", "error": "; ".join(errors)}


def translate_article(headline, summary="", source="auto", target="zh", preferred_api=None):
    detected = detect_language(headline)
    if detected == target:
        return {
            "success": True,
            "headline_zh": headline,
            "summary_zh": summary,
            "detected_lang": detected,
            "translator": "skip",
        }

    api = preferred_api or next_api()
    h_result = translate_single(headline, source, target, preferred_api=api)
    used_api = h_result.get("translator", api)

    s_result = None
    if summary and summary.strip():
        s_api = next_api() if not preferred_api else used_api
        s_result = translate_single(summary, source, target, preferred_api=s_api)

    return {
        "success": True,
        "headline_zh": h_result["translated"],
        "summary_zh": s_result["translated"] if s_result else "",
        "detected_lang": detected,
        "translator": used_api,
    }


def translate_batch(texts_json, source="auto", target="zh"):
    try:
        texts = json.loads(texts_json) if isinstance(texts_json, str) else texts_json
    except json.JSONDecodeError:
        return {"success": False, "error": "Invalid JSON"}

    translations = []
    for i, text in enumerate(texts):
        if isinstance(text, dict):
            t = translate_single(text.get("text", ""), source, target)
            t["id"] = text.get("id", "")
        else:
            t = translate_single(str(text), source, target)
        translations.append(t)

    return {
        "success": True,
        "translations": translations,
        "translator": "multi",
        "target_lang": target,
        "available_apis": available_apis(),
    }


def main(args=None):
    if args is None:
        args = sys.argv[1:]

    if len(args) < 2:
        print(json.dumps({"success": False,
                          "error": "Usage: translate_text.py <single|article|batch|detect> <text> [source] [target] [api]"}))
        return

    command = args[0]

    if command == "single":
        source = args[2] if len(args) > 2 else "auto"
        target = args[3] if len(args) > 3 else "zh"
        api = args[4] if len(args) > 4 else None
        result = {"success": True, **translate_single(args[1], source, target, preferred_api=api)}
    elif command == "article":
        headline = args[1]
        summary = ""
        source = "auto"
        target = "zh"
        api = None
        if len(args) > 2:
            nxt = args[2]
            if nxt in ("auto", "en", "zh", "ja", "ko"):
                source = nxt
            else:
                summary = nxt
        if len(args) > 3:
            nxt = args[3]
            if nxt in ("zh", "en", "ja", "ko"):
                target = nxt
            elif source == "auto" and nxt in ("auto", "en"):
                source = nxt
            else:
                summary = nxt if not summary else summary
        if len(args) > 4:
            target = args[4] if args[4] in ("zh", "en", "ja", "ko") else target
        if len(args) > 5:
            api = args[5]
        result = translate_article(headline, summary, source, target, preferred_api=api)
    elif command == "batch":
        source = args[2] if len(args) > 2 else "auto"
        target = args[3] if len(args) > 3 else "zh"
        result = translate_batch(args[1], source, target)
    elif command == "detect":
        lang = detect_language(args[1])
        result = {"success": True, "detected_lang": lang, "text": args[1][:100]}
    else:
        result = {"success": False, "error": f"Unknown command: {command}"}

    print(json.dumps(result, ensure_ascii=False))


if __name__ == "__main__":
    main()
