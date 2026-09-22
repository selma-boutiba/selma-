# Removing the future-work section: the seven references to repair

Deleting `\label{sec:future}` leaves seven `\ref` calls with nothing to point
at. Each will print `Section ??`. Below is every one, with replacement wording
that stands alone.

**Read the note at the end first** — there is an option that costs nothing and
loses nothing, and it fits your convention rather than breaking it.

---

## 1. Section 4.3.2 — `ch4_sec3_2_mad.tex`

**Was:**
> Determining the admissible fraction, and in particular the dependence on
> mask fragmentation that the argument above predicts, would require a sweep
> over occluder size and shape and is identified as future work in
> Section~\ref{sec:future}.

**Becomes:**
> Determining the admissible fraction, and in particular the dependence on
> mask fragmentation that the argument above predicts, would require a sweep
> over occluder size and shape, which is not attempted here.

---

## 2. Section 4.4.2 — `ch4_sec4_2_structure_measure_v2.tex`

**Was:**
> ...indicates that such an optimisation is warranted and it is identified as
> future work in Section~\ref{sec:future}.

**Becomes:**
> ...indicates that such an optimisation is warranted, though it is not
> attempted here.

---

## 3. Section 4.4.4 — `ch4_sec4_4_limitations.tex`

**Was:**
> Whether the normalisation reduces this over-rejection relative to a purely
> residual-based estimator is not established here, no weight map having been
> recorded for Variant~2, and is identified as an open point in
> Section~\ref{sec:future}.

**Becomes:**
> Whether the normalisation reduces this over-rejection relative to a purely
> residual-based estimator is not established here, no weight map having been
> recorded for Variant~2, and the question remains open.

---

## 4. Section 4.7.1 — `ch4_sec7_1_nominal_corrected.tex`

**Was:**
> ...establishing a ranking would require the study over several initial poses
> that Section~\ref{sec:future} identifies as outstanding.

**Becomes:**
> ...establishing a ranking would require a study over several initial poses,
> which is not undertaken here.

---

## 5. Section 4.7.2, first reference — `ch4_sec7_2_occlusion_written.tex`

**Was:**
> Establishing any ordering on accuracy would require the study over several
> initial poses identified in Section~\ref{sec:future}, and a weight map for
> Variant~2, which was not recorded.

**Becomes:**
> Establishing any ordering on accuracy would require a study over several
> initial poses, and a weight map for Variant~2, which was not recorded.

---

## 6. Section 4.7.2, second reference — the one that matters

This is the chapter's most valuable finding and it currently points forward to
a section that will no longer exist. The sentence works unchanged apart from
the reference.

**Was:**
> A gain that does not vanish with the error is accordingly identified in
> Section~\ref{sec:future} as the most immediate improvement available to the
> scheme, and the present chapter provides, inadvertently, the evidence for it.

**Becomes:**
> A gain that does not vanish with the error is accordingly the most immediate
> improvement available to the scheme, and the present chapter provides,
> inadvertently, the evidence for it.

---

## 7. Section 4.8 — `ch4_sec8_conclusion.tex`

The conclusion's last paragraph ends "the experiments needed to close it are
named", so if the reference goes, this sentence has to do the naming itself.

**Was:**
> Establishing any of this would require the study over several initial poses,
> and the further measurements, identified in Section~\ref{sec:future}.

**Becomes:**
> Establishing any of this would require a study over several initial poses,
> together with a weight map for the residual-based variant and a direct
> measurement of the overhead of the weighting, none of which is undertaken
> here.

---

# The option worth considering first

Your convention is a good one and I am not arguing with it: per-chapter
introduction and conclusion, with no future work inside a chapter, is standard
practice. But a thesis has perspectives *somewhere*, almost always in the
general conclusion, and that is where this material belongs rather than in the
bin.

**Move `ch4_sec9_future_work.tex` into the general conclusion and keep the
label `sec:future` on it.** Then:

- all seven references resolve, now pointing forward to the final chapter,
  which reads *better* than pointing sideways within Chapter 4;
- none of the seven sentences above needs touching;
- your chapter structure is exactly the one you want — Chapter 4 ends at its
  conclusion;
- and nothing is lost.

What you lose by deleting outright is not the *findings* — those are already
stated in Section 4.8 and survive on their own — but the *proposals*: the
three candidate gain schedules, the comparison of the structure measure
against the reference image, the annealing of the Tukey constant, the design
of the occlusion-fraction sweep, the correlated-residuals misspecification,
and the criteria a learned weighting would have to meet. Those are the answers
to "so what would you do next?", which is the question a jury asks when the
defence is nearly over, and they are worth having written down.

If the general conclusion already has its own perspectives section, merge this
file into it rather than adding a second one — the two would overlap heavily
on the multi-pose study.
