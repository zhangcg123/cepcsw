# Overshoot-19 reverse versus CMSSW-like comparison (2026-07-15)

At the user's direction the KL smoother was omitted. The durable 19-event
topology-clean light-eBrem transition-5--11 overshoot population was rerun
same-code at MaxComponents=24 with the default five-component CEPC BH model,
once with ordinary reverse BestBranch and once with `CmsGsfSmoothing`. Every
event completed in both workflows with identical hit counts method-to-method.

| method | median residual | mean absolute | RMS | central-68 half-width | inside 1% | inside 2% |
|---|---:|---:|---:|---:|---:|---:|
| LCIO | -3.2248% | 4.0741% | 4.6760% | 2.3739% | 0/19 | 3/19 |
| reverse | +1.4097% | 2.0871% | 2.6872% | 0.7390% | 1/19 | 14/19 |
| CMSSW-like | +1.6114% | 1.8199% | 1.9726% | 0.7994% | 3/19 | 13/19 |

CMSSW-like has lower absolute truth error in 9/19 and reverse in 10/19. It
publishes lower pT than reverse in 8 events and higher pT in 11, so it is not a
uniform damping of overshoot. Its aggregate MAE/RMS improve mainly because it
partially recovers the current identity-like 469/6 failure (-8.46% reverse to
-3.21% CMSSW-like) and reduces several large positive overshoots, including
65/6 (+2.52% to +1.71%), 102/4 (+2.79% to +2.02%), and 443/2 (+3.62% to
+2.72%). It also worsens cases including 371/5 (+1.13% to +2.49%) and 272/0
(+1.85% to +2.70%).

Outputs and per-seed logs are under `/tmp/gsf-overshoot19-reverse24` and
`/tmp/gsf-overshoot19-cms24`. The sample runner now accepts `--workflow cms`.
Future routine comparisons should omit the KL smoother unless explicitly
requested, per the user's instruction.
