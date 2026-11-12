#!/usr/bin/env python3
"""One-time Spotify login. Run on your computer, not the ESP32.

Uses the PKCE flow, so no Client Secret is needed (and none ever goes on the board).
Prints a refresh token that you paste into include/secrets.h.

Usage:
    python3 tools/spotify_login.py --client-id <your-client-id>
    SPOTIFY_CLIENT_ID=<your-client-id> python3 tools/spotify_login.py

The redirect URI http://127.0.0.1:8888/callback must be registered in your
Spotify app's settings.
"""

import argparse
import base64
import hashlib
import http.server
import json
import os
import secrets
import threading
import urllib.error
import urllib.parse
import urllib.request
import webbrowser

REDIRECT_URI = "http://127.0.0.1:8888/callback"   # must match the dashboard exactly
SCOPES = "user-read-currently-playing user-read-playback-state"

AUTH_URL = "https://accounts.spotify.com/authorize"
TOKEN_URL = "https://accounts.spotify.com/api/token"


def make_pkce_pair():
    verifier = base64.urlsafe_b64encode(secrets.token_bytes(64)).rstrip(b"=").decode()
    digest = hashlib.sha256(verifier.encode()).digest()
    challenge = base64.urlsafe_b64encode(digest).rstrip(b"=").decode()
    return verifier, challenge


class CallbackHandler(http.server.BaseHTTPRequestHandler):
    result = {}

    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        if parsed.path != "/callback":
            self.send_response(404)
            self.end_headers()
            return
        CallbackHandler.result = dict(urllib.parse.parse_qsl(parsed.query))
        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.end_headers()
        ok = "code" in CallbackHandler.result
        msg = "Login complete. You can close this tab and go back to the terminal." if ok \
            else "Login failed. Go back to the terminal for details."
        self.wfile.write(f"<html><body style='font-family:sans-serif;padding:2em'><h2>{msg}</h2></body></html>".encode())

    def log_message(self, *args):
        pass  # keep the terminal clean


def exchange_code(client_id, code, verifier):
    body = urllib.parse.urlencode({
        "client_id": client_id,
        "grant_type": "authorization_code",
        "code": code,
        "redirect_uri": REDIRECT_URI,
        "code_verifier": verifier,
    }).encode()
    req = urllib.request.Request(TOKEN_URL, data=body, method="POST",
                                 headers={"Content-Type": "application/x-www-form-urlencoded"})
    try:
        with urllib.request.urlopen(req, timeout=20) as resp:
            return json.load(resp)
    except urllib.error.HTTPError as e:
        raise SystemExit(f"Token exchange failed ({e.code}): {e.read().decode()}")


def check_now_playing(access_token):
    req = urllib.request.Request("https://api.spotify.com/v1/me/player/currently-playing",
                                 headers={"Authorization": f"Bearer {access_token}"})
    try:
        with urllib.request.urlopen(req, timeout=20) as resp:
            if resp.status == 204:
                return "Nothing is playing right now (that's fine)."
            data = json.load(resp)
            item = data.get("item") or {}
            artists = ", ".join(a["name"] for a in item.get("artists", []))
            return f"Now playing: {item.get('name', '?')} by {artists or '?'}"
    except urllib.error.HTTPError as e:
        return f"Now-playing check returned HTTP {e.code}: {e.read().decode()[:300]}"


def parse_args():
    parser = argparse.ArgumentParser(description="Obtain a Spotify refresh token (PKCE).")
    parser.add_argument("--client-id", default=os.environ.get("SPOTIFY_CLIENT_ID"),
                        help="Spotify app client ID (or set SPOTIFY_CLIENT_ID)")
    args = parser.parse_args()
    if not args.client_id:
        parser.error("a client ID is required (--client-id or SPOTIFY_CLIENT_ID)")
    return args


def main():
    client_id = parse_args().client_id
    verifier, challenge = make_pkce_pair()
    state = secrets.token_urlsafe(16)

    params = urllib.parse.urlencode({
        "client_id": client_id,
        "response_type": "code",
        "redirect_uri": REDIRECT_URI,
        "scope": SCOPES,
        "code_challenge_method": "S256",
        "code_challenge": challenge,
        "state": state,
    })
    url = f"{AUTH_URL}?{params}"

    server = http.server.HTTPServer(("127.0.0.1", 8888), CallbackHandler)
    thread = threading.Thread(target=server.handle_request)
    thread.start()

    print("Opening your browser to log in to Spotify...")
    print("If it doesn't open, paste this URL into your browser:\n")
    print(url + "\n")
    webbrowser.open(url)

    thread.join(timeout=300)
    server.server_close()

    res = CallbackHandler.result
    if not res:
        raise SystemExit("Timed out waiting for login (5 minutes). Run the script again.")
    if res.get("state") != state:
        raise SystemExit("State mismatch. Run the script again.")
    if "error" in res:
        raise SystemExit(f"Spotify returned an error: {res['error']}")

    tokens = exchange_code(client_id, res["code"], verifier)
    refresh = tokens.get("refresh_token")
    if not refresh:
        raise SystemExit(f"No refresh token in response: {tokens}")

    print("\n" + check_now_playing(tokens["access_token"]))
    print("\n=== REFRESH TOKEN (keep private; paste it into include/secrets.h) ===")
    print(refresh)
    print("==============================================================")


if __name__ == "__main__":
    main()
