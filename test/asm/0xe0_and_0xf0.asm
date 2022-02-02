SECTION "Header", ROM0[$100]
    jp Entry

    ;padding
    ds $150 - @, 0

Entry:
    ld a, $55
    ldh [$ff01], a
    ld a, $01
    ldh a, [$ff01]

