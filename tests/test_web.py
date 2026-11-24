import json
from pathlib import Path
import threading
import unittest
from urllib.request import Request, urlopen
from urllib.error import HTTPError
from http.server import ThreadingHTTPServer

from web_server import analyze, handler_for

ROOT = Path(__file__).resolve().parents[1]
FIXTURE = ROOT / "tests" / "fixture_patterns.txt"
HAND = "F F 5B 5B 5B 6B 6B 6B 7B 7B 7B 8B 8B E".split()


class BrowserTests(unittest.TestCase):
    def test_draws_and_discards(self):
        result = analyze({"hand": HAND}, patterns=FIXTURE)
        self.assertEqual(result["mode"], "discards")
        self.assertEqual(result["choices"][0]["discard"], "E")
        result = analyze({"hand": HAND[:-1], "visible": ["8B", "J"]}, patterns=FIXTURE)
        self.assertEqual(result["mode"], "draws")
        self.assertEqual(result["improving_copies"], 8)

    def test_lookahead(self):
        result = analyze({"hand": HAND[:-1], "lookahead": True}, patterns=FIXTURE)
        self.assertEqual(result["one_draw_completion_copies"], 10)
        self.assertEqual(result["two_draw_successful_sequences"], 139*138 - 129*128)
        result = analyze({"hand": HAND, "lookahead": True}, patterns=FIXTURE)
        self.assertEqual(result["choices"][0]["discard"], "E")
        self.assertNotIn("two_draw_completion_percentage",
                         analyze({"hand": HAND[:-1]}, patterns=FIXTURE))
        with self.assertRaises(ValueError):
            analyze({"hand": HAND, "lookahead": "true"}, patterns=FIXTURE)

    def test_bad_requests(self):
        for payload in ({}, {"hand": HAND, "visible": ["8B"] * 3},
                        {"hand": HAND[:-1] + ["--json"]},
                        {"hand": HAND, "visible": [{}]}, []):
            with self.subTest(payload=payload), self.assertRaises(ValueError):
                analyze(payload, patterns=FIXTURE)

    def test_http_interface(self):
        server = ThreadingHTTPServer(("127.0.0.1", 0), handler_for(FIXTURE))
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        url = f"http://127.0.0.1:{server.server_port}"
        try:
            with urlopen(url + "/", timeout=5) as response:
                self.assertIn(b"American Mahjong Analyzer", response.read())
            req = Request(url + "/api/analyze", data=json.dumps({"hand": HAND}).encode(),
                          headers={"Content-Type": "application/json"})
            with urlopen(req, timeout=20) as response:
                self.assertEqual(json.load(response)["choices"][0]["discard"], "E")
            req = Request(url + "/api/analyze", data=b"{}",
                          headers={"Content-Type": "application/json"})
            with self.assertRaises(HTTPError) as caught:
                urlopen(req, timeout=5)
            self.assertEqual(caught.exception.code, 400)
            req = Request(url + "/api/analyze", data=b"{}",
                          headers={"Content-Type": "application/json", "Origin": "https://example.com"})
            with self.assertRaises(HTTPError) as caught:
                urlopen(req, timeout=5)
            self.assertEqual(caught.exception.code, 403)
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=5)


if __name__ == "__main__":
    unittest.main()
