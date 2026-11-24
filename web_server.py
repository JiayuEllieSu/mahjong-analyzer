"""Local browser interface for the C++ American Mahjong analyzer."""
import argparse
from collections import Counter
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent
TILES = {f"{rank}{suit}" for suit in "BDC" for rank in range(1, 10)}
TILES.update(("E", "S", "W", "N", "RD", "GD", "WD", "F", "J"))


def analyze(payload, binary=ROOT / "mahjong", patterns=ROOT / "demo_patterns.txt"):
    """Validate a browser request, then run the existing engine without a shell."""
    if not isinstance(payload, dict):
        raise ValueError("Expected a JSON object.")
    hand = payload.get("hand")
    visible = payload.get("visible", [])
    lookahead = payload.get("lookahead", False)
    if not isinstance(lookahead, bool):
        raise ValueError("lookahead must be a boolean.")
    if not isinstance(hand, list) or len(hand) not in (13, 14):
        raise ValueError("Enter exactly 13 or 14 hand tiles.")
    if not isinstance(visible, list) or len(visible) > 152:
        raise ValueError("Invalid visible tile list.")
    for tile in hand + visible:
        if not isinstance(tile, str) or tile not in TILES:
            raise ValueError("Invalid tile code.")
    for tile, count in Counter(hand + visible).items():
        if count > (8 if tile in ("F", "J") else 4):
            raise ValueError(f"Too many copies of {tile}.")
    command = [str(binary), "--patterns", str(patterns), "--json", "--all",
               "--hand", " ".join(hand)]
    if len(hand) == 13:
        command.append("--draws")
    if lookahead:
        command.append("--lookahead")
    if visible:
        command.extend(("--visible", " ".join(visible)))
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                            timeout=15, check=False)
    if result.returncode:
        raise ValueError(result.stderr.strip() or "Analyzer rejected the hand.")
    return json.loads(result.stdout)


def handler_for(patterns, binary=ROOT / "mahjong"):
    class Handler(BaseHTTPRequestHandler):
        def send_body(self, status, body, content_type):
            self.send_response(status)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.send_header("X-Content-Type-Options", "nosniff")
            self.end_headers()
            self.wfile.write(body)

        def send_json(self, status, obj):
            self.send_body(status, json.dumps(obj).encode(), "application/json")

        def local_request(self):
            port = self.server.server_port
            hosts = {f"127.0.0.1:{port}", f"localhost:{port}"}
            host = self.headers.get("Host")
            origin = self.headers.get("Origin")
            return host in hosts and (origin is None or origin == f"http://{host}")

        def do_GET(self):
            if not self.local_request():
                self.send_json(403, {"error": "Local requests only."})
            elif self.path == "/":
                self.send_body(200, (ROOT / "web" / "index.html").read_bytes(),
                               "text/html; charset=utf-8")
            else:
                self.send_json(404, {"error": "Not found."})

        def do_POST(self):
            if not self.local_request():
                self.send_json(403, {"error": "Local requests only."})
                return
            if self.path != "/api/analyze":
                self.send_json(404, {"error": "Not found."})
                return
            if self.headers.get_content_type() != "application/json":
                self.send_json(415, {"error": "Expected application/json."})
                return
            try:
                size = int(self.headers.get("Content-Length", "0"))
                if not 0 < size <= 8192:
                    raise ValueError("Invalid request size.")
                payload = json.loads(self.rfile.read(size))
                result = analyze(payload, binary, patterns)
            except (ValueError, UnicodeError) as exc:
                self.send_json(400, {"error": str(exc)})
                return
            except (OSError, subprocess.TimeoutExpired):
                self.send_json(503, {"error": "Analyzer unavailable; run make first."})
                return
            self.send_json(200, result)

    return Handler


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--patterns", type=Path, default=ROOT / "demo_patterns.txt")
    args = parser.parse_args()
    patterns = args.patterns.resolve()
    if not (ROOT / "mahjong").is_file():
        parser.error("Run make before starting the browser interface.")
    if not patterns.is_file():
        parser.error("Pattern file does not exist.")
    with ThreadingHTTPServer(("127.0.0.1", args.port), handler_for(patterns)) as server:
        print(f"Open http://127.0.0.1:{server.server_port} (Ctrl+C to stop)", flush=True)
        print(f"Patterns: {patterns.name}", flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
