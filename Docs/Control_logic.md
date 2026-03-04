# Control Logic

## Light Tracking

If Left LDR > Right LDR:
    Rotate motor left

If Right LDR > Left LDR:
    Rotate motor right

Movement occurs only if difference exceeds threshold to avoid jitter.

---

## Wind Protection

If wind speed exceeds defined limit:
    Move reflector to safe horizontal position.
