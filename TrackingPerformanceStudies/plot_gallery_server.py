"""Serve a plot directory with a self-updating PNG gallery at /.

Uses only the Python standard library. Bound to localhost by design; forward
the port through SSH or VS Code when viewing from another machine.
"""

import argparse
from functools import partial
from html import escape
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import quote, urlsplit


def gallery(root):
    images = sorted(
        (path for path in root.rglob("*") if path.is_file() and path.suffix.lower() == ".png"),
        key=lambda path: str(path.relative_to(root)).lower(),
    )
    cards = []
    for path in images:
        relative = path.relative_to(root).as_posix()
        url = "/" + quote(relative, safe="/")
        label = escape(relative)
        cards.append(
            f'<a class="card" href="{url}" target="_blank" rel="noopener" '
            f'data-name="{escape(relative.lower(), quote=True)}">'
            f'<img src="{url}" alt="{label}" loading="lazy">'
            f'<span>{label}</span></a>'
        )
    links = []
    for name in ("README.md", "resolution_summary.csv", "category_counts.csv", "location_resolution.csv"):
        if (root / name).is_file():
            links.append(f'<a href="/{quote(name)}">{escape(name)}</a>')
    return (
        "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">"
        '<meta name="viewport" content="width=device-width,initial-scale=1">'
        '<title>Plot gallery</title><style>'
        'body{font:16px system-ui,sans-serif;margin:0;background:#f5f7fa;color:#17212b}'
        'header{position:sticky;top:0;background:#fff;border-bottom:1px solid #ddd;padding:1rem 1.5rem;z-index:1}'
        'h1{font-size:1.4rem;margin:0 0 .5rem}p{margin:.35rem 0;color:#4b5563}'
        'input{font:inherit;padding:.55rem;width:min(38rem,95%);border:1px solid #9ca3af;border-radius:6px}'
        'nav{display:flex;gap:1rem;flex-wrap:wrap;margin-top:.5rem}'
        'main{display:grid;grid-template-columns:repeat(auto-fill,minmax(310px,1fr));gap:1rem;padding:1rem}'
        '.card{background:white;border:1px solid #ddd;border-radius:8px;overflow:hidden;'
        'text-decoration:none;color:inherit;box-shadow:0 1px 3px #0001}'
        '.card:hover{outline:2px solid #377dca}'
        'img{width:100%;height:245px;object-fit:contain;background:#fff;display:block}'
        'span{display:block;padding:.7rem;overflow-wrap:anywhere;font-size:.88rem}'
        '</style></head><body><header><h1>Plot gallery</h1>'
        f'<p>{len(images)} PNG plots in {escape(str(root))}. Click a preview for the full image.</p>'
        '<input id="filter" aria-label="Filter plot names" placeholder="Filter plot names…">'
        f'<nav>{" ".join(links)}</nav></header><main id="gallery">'
        + "".join(cards)
        + '</main><script>document.getElementById("filter").addEventListener("input",e=>{'
          'const q=e.target.value.toLowerCase();document.querySelectorAll(".card").forEach(c=>'
          'c.hidden=!c.dataset.name.includes(q))})</script></body></html>'
    ).encode("utf-8")


class GalleryHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        if urlsplit(self.path).path in ("/", "/index.html"):
            content = gallery(Path(self.directory))
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)
            return
        super().do_GET()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path, help="directory containing PNG plots")
    parser.add_argument("--port", type=int, default=8000)
    args = parser.parse_args()
    root = args.directory.expanduser().resolve(strict=True)
    if not root.is_dir():
        parser.error(f"not a directory: {root}")
    handler = partial(GalleryHandler, directory=str(root))
    with ThreadingHTTPServer(("127.0.0.1", args.port), handler) as server:
        print(f"Plot gallery: http://127.0.0.1:{args.port}/ -> {root}", flush=True)
        server.serve_forever()


if __name__ == "__main__":
    main()
