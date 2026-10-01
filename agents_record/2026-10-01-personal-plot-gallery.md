# Personal plot-gallery skill

The user requested the existing plot-gallery workflow as a reusable personal
Codex skill for sessions in different projects and on two remote Linux hosts.

Installed on the current host under `~/.agents/skills/plot-gallery/`:

- `SKILL.md`: requests one sub-agent for directory inspection, meaningful
  manifest categories, validation, server handoff, launch and HTTP checks.
- `scripts/gallery.py`: generic Python gallery, using the local `gallery.json`.
- `scripts/plotweb.py`: validate/start/status/stop, localhost-only, configurable
  port (default 8000), checked process ownership, serialized lifecycle commands,
  host-specific state and PID-namespace checks.
- `references/install.md`: copying to another host and SSH forwarding.

`~/.codex/AGENTS.md` now asks for this personal skill and delegation. The user's
existing `plotweb_start`, `plotweb_status` and `plotweb_stop` shell helpers use
the installed skill. The previous `.bashrc` is preserved at
`~/.bashrc.plot-gallery-before-20261001`. No existing plot image or gallery
manifest was modified. The project script remains a historical copy.

Validation: skill format and Python checks passed. The existing breakpoint
gallery manifest covers 40 PNGs exactly once in six sections. Live verification
checked the rendered 40 previews and an image response; non-image file requests
were denied for GET and HEAD. Start/status/stop passed on test port 18880.
Local HTTP checks explicitly bypass shell proxies. Final deployment transferred
the verified previous breakpoint gallery from its project script to the personal
launcher on localhost port 8000, retaining the same directory.

Each host owns its own web process. Shared home directories are supported by
keeping runtime files under `~/.local/state/plot-gallery/<hostname>/` (or the
configured XDG state root). Lifecycle checks must run in the host's process
and network context, not inside a namespace that hides its server PID.

The skill has no CEPCSW path dependency. The second Linux host has not been
accessed or installed; the packaged instructions explain the copy/setup there.
