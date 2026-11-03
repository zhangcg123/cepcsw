# Separate-neutral stage off by default

User requested111 OFF by default. Changed the ordinary submission helper's
BP_NEUTRAL_ECAL_MEASUREMENT fallback from1 to0, making its default110. Standalone
run card and compiled NeutralEcalMeasurement already default false; no C++ or
installed-library update is needed for this steering change.

Passive neutral collection and110 ElectronPhoton's resolved charged+neutral sum
remain available. Disabled111 is not run; its collections are empty and endpoint
fields remain NaN. Explicit BP_NEUTRAL_ECAL_MEASUREMENT=1 still enables it.
The dedicated four-setup HitScanAll launcher already explicitly sets1 and keeps
that intentional campaign override. Existing frozen cards are not rewritten.

Wrapper tests now check default110, explicit111, and explicit101. All59 Python
batch tests pass, including the dedicated scan launcher's explicit111 steering.
Both shell syntax checks and diff whitespace checks pass. No batch submitted.
No C++/installation operation for this task; the separate ElectronOnly routing
correction remains staged/not installed, as recorded in the preceding handoff.
