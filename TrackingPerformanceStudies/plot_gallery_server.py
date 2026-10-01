"""Serve a plot directory with a self-updating PNG gallery at /.

Uses only the Python standard library. Bound to localhost by design; forward
the port through SSH or VS Code when viewing from another machine. Optional
gallery.json holds human-authored title, intro and ordered sections; the server
never infers plot meanings or physics categories. Unassigned new images remain
visible in a separate section.
"""

import argparse
import fnmatch
import json
from functools import partial
from html import escape
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import quote, urlsplit


def sections_for(root, images):
    """Apply optional human-authored categories; never infer physics from names."""
    manifest_path = root / "gallery.json"
    if not manifest_path.is_file():
        return "Plot gallery", "", [("All plots", "", images)]
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    if manifest.get("schema_version") != 1 or not isinstance(manifest.get("sections"), list):
        raise ValueError("gallery.json requires schema_version=1 and a sections list")
    title = str(manifest.get("title", "Plot gallery"))
    intro = str(manifest.get("intro", ""))
    remaining = {path.relative_to(root).as_posix(): path for path in images}
    groups = []
    for section in manifest["sections"]:
        if not isinstance(section, dict) or not isinstance(section.get("patterns"), list):
            raise ValueError("each gallery section requires a patterns list")
        heading = str(section.get("title", "Untitled section"))
        description = str(section.get("description", ""))
        chosen = []
        # Pattern order is the display order; each matched filename sorts within
        # its pattern. First section wins if patterns overlap.
        for pattern in section["patterns"]:
            for relative in sorted(list(remaining)):
                if fnmatch.fnmatchcase(relative, str(pattern)):
                    chosen.append(remaining.pop(relative))
        if chosen:
            groups.append((heading, description, chosen))
    if remaining:
        groups.append(("New / uncategorized", "Plots not yet assigned in gallery.json.",
                       [remaining[name] for name in sorted(remaining)]))
    return title, intro, groups


def card(root, path):
    relative = path.relative_to(root).as_posix()
    url = "/" + quote(relative, safe="/")
    label = escape(relative)
    return (
        f'<a class="card" href="{url}" target="_blank" rel="noopener" '
        f'data-name="{escape(relative.lower(), quote=True)}">'
        f'<img src="{url}" alt="{label}" loading="lazy">'
        f'<span>{label}</span></a>'
    )


def gallery(root):
    images = sorted(
        (path for path in root.rglob("*") if path.is_file() and path.suffix.lower() == ".png"),
        key=lambda path: str(path.relative_to(root)).lower(),
    )
    title, intro, sections = sections_for(root, images)
    links = ['<a href="/README.md">README.md</a>'] if (root / "README.md").is_file() else []
    section_html = []
    navigation = []
    for index, (heading, description, paths) in enumerate(sections):
        anchor = f"section-{index + 1}"
        navigation.append(f'<a href="#{anchor}">{escape(heading)} ({len(paths)})</a>')
        section_html.append(
            f'<section class="group" id="{anchor}"><h2>{escape(heading)}</h2>'
            f'<p>{escape(description)}</p><div class="grid">'
            + "".join(card(root, path) for path in paths) + "</div></section>"
        )
    return (
        "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">"
        '<meta name="viewport" content="width=device-width,initial-scale=1">'
        f'<title>{escape(title)}</title><style>'
        'body{font:16px system-ui,sans-serif;margin:0;background:#f5f7fa;color:#17212b}'
        'header{background:#fff;border-bottom:1px solid #ddd;padding:1rem 1.5rem}'
        'h1{font-size:1.4rem;margin:0 0 .5rem}h2{margin:.25rem 0 .5rem}'
        'p{margin:.35rem 0;color:#4b5563}'
        'input{font:inherit;padding:.55rem;width:min(38rem,95%);border:1px solid #9ca3af;border-radius:6px}'
        'nav{display:flex;gap:1rem;flex-wrap:wrap;margin-top:.5rem}'
        'main{padding:1rem}.group{margin:0 0 2.5rem}.group h2{border-bottom:2px solid #d4dce4;padding-bottom:.3rem}'
        '.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(310px,1fr));gap:1rem;margin-top:1rem}'
        '.card{background:white;border:1px solid #ddd;border-radius:8px;overflow:hidden;'
        'text-decoration:none;color:inherit;box-shadow:0 1px 3px #0001}'
        '.card:hover{outline:2px solid #377dca}'
        'img{width:100%;height:245px;object-fit:contain;background:#fff;display:block}'
        'span{display:block;padding:.7rem;overflow-wrap:anywhere;font-size:.88rem}'
        '</style></head><body><header>'
        f'<h1>{escape(title)}</h1><p>{escape(intro)}</p>'
        f'<p>{len(images)} PNG plots. Click a preview for the full image.</p>'
        '<input id="filter" aria-label="Filter plot names" placeholder="Filter plot names…">'
        f'<nav>{" ".join(navigation + links)}</nav></header><main id="gallery">'
        + "".join(section_html)
        + '</main><script>document.getElementById("filter").addEventListener("input",e=>{'
          'const q=e.target.value.toLowerCase();document.querySelectorAll(".group").forEach(g=>{'
          'let visible=0;g.querySelectorAll(".card").forEach(c=>{'
          'c.hidden=!c.dataset.name.includes(q);if(!c.hidden)visible++});g.hidden=!visible})'
          '})</script></body></html>'
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
