# RecBreakpoint score-breakdown deployment

User explicitly requested installation of the previously verified default-on
measurement/process/seed chi2 serialization update. No local Gaudi or
breakpoint submission process was active. No tracker input ROOT remained after
the simulation-only tuple cleanup, so no new event-level fit was run during
deployment; the event-level A/B gate is recorded in
`2026-10-01-breakpoint-default-score-breakdown.md` and the 50-event study in
`2026-10-01-breakpoint-score-more50.md`.

The five prior installed RecBreakpoint artifacts were archived to
`/tmp/recbreakpoint-deploy-20261001.CTgTYJ/installed-before.tar` before the
package-only CMake install script ran. This is a temporary local rollback
archive, not a Git-tracked release artifact. Only the RecBreakpoint package
install script was invoked; no repository-wide install or source/card edit.

Before deployment, installed `libRecBreakpoint.so` SHA256 was
`8689b40e378ce8886e7e0784de983a17c0d943be2b3d014939106d0a73482b0d`.
After deployment, installed and tested build SHA256 both equal
`c8920c0b5ba7339b5e8e5494f2b097f3d91681811841caa46e49c05ed04dfe53`.
The installed configurable imports and ROOT's explicit plugin loader returned
success (status 0). Previous local before/after tests reproduced all existing
fields and component/state records, and verified new score sum/copy/fallback
behavior. The new fields are now available to normal RecBreakpoint jobs without
changing the maintained run card. No remote or GSF/shared-KF runtime was
changed by the installation.
