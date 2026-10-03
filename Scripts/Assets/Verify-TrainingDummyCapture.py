"""PIE 자동 검사와 허수아비 전용 submix 녹음 검증; Python 표준 라이브러리만 사용"""
import array
import json
import math
import sys
import wave
from pathlib import Path

root = Path(__file__).resolve().parents[2] / "Saved/AssetValidation/TrainingDummy"
automation = json.loads((root / "Automation/index.json").read_text(encoding="utf-8-sig"))
test = next(t for t in automation["tests"] if t["fullTestPath"] == "Maverick.Combat.TrainingDummy.PlayerMeleePIE")
with wave.open(str(root / "training-hit-output.wav"), "rb") as recording:
    assert recording.getsampwidth() == 2, "Expected PCM16 engine output"
    channels, rate, frames = recording.getnchannels(), recording.getframerate(), recording.getnframes()
    samples = array.array("h", recording.readframes(frames))
if sys.byteorder != "little":
    samples.byteswap()
peak = max((abs(v) for v in samples), default=0) / 32768
rms = math.sqrt(sum((v / 32768) ** 2 for v in samples) / max(1, len(samples)))
window = max(1, int(rate * channels * .1))
active = [max((abs(v) for v in samples[i:i + window]), default=0) > 32 for i in range(0, len(samples), window)]
bursts = sum(value and (index == 0 or not active[index - 1]) for index, value in enumerate(active))
result = {"test": test["fullTestPath"], "state": test["state"], "engine_output": "training-hit-output.wav",
          "channels": channels, "sample_rate": rate, "duration_seconds": frames / rate,
          "peak": peak, "rms": rms, "audible_window_groups": bursts,
          "scope": "Actual player melee in PIE; isolated dummy hit submix output, not source WAV or speaker listening"}
result["passed"] = test["state"] == "Success" and peak > .001 and rms > .0001 and bursts >= 2
(root / "capture-validation.json").write_text(json.dumps(result, indent=2), encoding="utf-8")
print(json.dumps(result, indent=2))
raise SystemExit(0 if result["passed"] else 1)
