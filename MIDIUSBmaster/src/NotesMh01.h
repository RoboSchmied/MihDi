#define C8	  108

#define B7  107
#define B7b 106
#define A7  105
#define A7b 104
#define G7  103
#define G7b 102
#define F7  101
#define E7  100
#define E7b 99
#define D7  98
#define D7b 97
#define C7  96

#define B6  95
#define B6b 94
#define A6  93
#define A6b 92
#define G6  91
#define G6b 90
#define F6  89
#define E6  88
#define E6b 87
#define D6  86
#define D6b 85
#define C6  84

#define B5  83
#define B5b 82
#define A5  81
#define A5b 80
#define G5  79
#define G5b 78
#define F5  77
#define E5  76
#define E5b 75
#define D5  74
#define D5b 73
#define C5  72

#define B4  71
#define B4b 70
#define A4  69
#define A4b 68
#define G4  67
#define G4b 66
#define F4  65
#define E4  64
#define E4b 63
#define D4  62
#define D4b 61
#define C4  60

#define B3  59
#define B3b 58
#define A3  57
#define A3b 56
#define G3  55
#define G3b 54
#define F3  53
#define E3  52
#define E3b 51
#define D3  50
#define D3b 49
#define C3  48

#define B2  47
#define B2b 46
#define A2  45
#define A2b 44
#define G2  43
#define G2b 42
#define F2  41
#define E2  40
#define E2b 39
#define D2  38
#define D2b 37
#define C2  36

#define B1Eins  35
#define B1b 34
#define A1  33
#define A1b 32
#define G1  31
#define G1b 30
#define F1  29
#define E1  28
#define E1b 27
#define D1  26
#define D1b 25
#define C1  24

#define B0Null  23 
#define B0b 22
#define A0  21

#define A0b 20  ///added by mh
#define G0  19
#define G0b 18
#define F0  17
#define E0  16
#define E0b 15
#define D0  14
#define D0b 13
#define C0  12




/// Full (88) 97 key range note  high to low midi byte value mapping array
const byte note[] = {C8,
                               B7, B7b, A7, A7b, G7, G7b, F7, E7, E7b, D7, D7b, C7,
                               B6, B6b, A6, A6b, G6, G6b, F6, E6, E6b, D6, D6b, C6,
                               B5, B5b, A5, A5b, G5, G5b, F5, E5, E5b, D5, D5b, C5,
                               B4, B4b, A4, A4b, G4, G4b, F4, E4, E4b, D4, D4b, C4,
                               B3, B3b, A3, A3b, G3, G3b, F3, E3, E3b, D3, D3b, C3,
                               B2, B2b, A2, A2b, G2, G2b, F2, E2, E2b, D2, D2b, C2,
                               B1Eins, B1b, A1, A1b, G1, G1b, F1, E1, E1b, D1, D1b, C1,
                               B0Null, B0b, A0, A0b, G0, G0b, F0, E0, E0b, D0, D0b, C0};

/// 36 schwarze Tasten  fuer einfache Harmonie
///const byte noteMh[] = { B7b, A7b, G7b, E7b, D7b,
///                        B6b, A6b, G6b, E6b, D6b,
///                        B5b, A5b, G5b, E5b, D5b,
///                        B4b, A4b, G4b, E4b, D4b,
///                        B3b, A3b, G3b, E3b, D3b,
///                        B2b, A6b, G2b, E2b, D2b,
///                        B1b, A1b, G1b, E1b, D1b,
///                        B0b};


/// 40 schwarze Tasten  fuer einfache Harmonie
const byte noteMh[] = { B7b, A7b, G7b, E7b, D7b,
                        B6b, A6b, G6b, E6b, D6b,
                        B5b, A5b, G5b, E5b, D5b,
                        B4b, A4b, G4b, E4b, D4b,
                        B3b, A3b, G3b, E3b, D3b,
                        B2b, A2b, G2b, E2b, D2b,
                        B1b, A1b, G1b, E1b, D1b,
                        B0b, A0b, G0b, E0b, D0b};
