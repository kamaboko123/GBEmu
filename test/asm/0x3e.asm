SECTION "Header", ROM0[$100]
    jp Entry

    ;padding
    ds $150 - @, 0

Entry:
    ld a, $55

