extern unsigned long long const rook_magic_mask[64] = {
	0x000101010101017eull,
	0x000202020202027cull,
	0x000404040404047aull,
	0x0008080808080876ull,
	0x001010101010106eull,
	0x002020202020205eull,
	0x004040404040403eull,
	0x008080808080807eull,
	0x0001010101017e00ull,
	0x0002020202027c00ull,
	0x0004040404047a00ull,
	0x0008080808087600ull,
	0x0010101010106e00ull,
	0x0020202020205e00ull,
	0x0040404040403e00ull,
	0x0080808080807e00ull,
	0x00010101017e0100ull,
	0x00020202027c0200ull,
	0x00040404047a0400ull,
	0x0008080808760800ull,
	0x00101010106e1000ull,
	0x00202020205e2000ull,
	0x00404040403e4000ull,
	0x00808080807e8000ull,
	0x000101017e010100ull,
	0x000202027c020200ull,
	0x000404047a040400ull,
	0x0008080876080800ull,
	0x001010106e101000ull,
	0x002020205e202000ull,
	0x004040403e404000ull,
	0x008080807e808000ull,
	0x0001017e01010100ull,
	0x0002027c02020200ull,
	0x0004047a04040400ull,
	0x0008087608080800ull,
	0x0010106e10101000ull,
	0x0020205e20202000ull,
	0x0040403e40404000ull,
	0x0080807e80808000ull,
	0x00017e0101010100ull,
	0x00027c0202020200ull,
	0x00047a0404040400ull,
	0x0008760808080800ull,
	0x00106e1010101000ull,
	0x00205e2020202000ull,
	0x00403e4040404000ull,
	0x00807e8080808000ull,
	0x007e010101010100ull,
	0x007c020202020200ull,
	0x007a040404040400ull,
	0x0076080808080800ull,
	0x006e101010101000ull,
	0x005e202020202000ull,
	0x003e404040404000ull,
	0x007e808080808000ull,
	0x7e01010101010100ull,
	0x7c02020202020200ull,
	0x7a04040404040400ull,
	0x7608080808080800ull,
	0x6e10101010101000ull,
	0x5e20202020202000ull,
	0x3e40404040404000ull,
	0x7e80808080808000ull
};

extern unsigned long long const rook_magic_multiplier[64] = {
	0x8080008040002010ull,
	0x4180200040008010ull,
	0x8100104100200008ull,
	0xa080080010000480ull,
	0xc100080100021004ull,
	0x4100040002088100ull,
	0x0280010002000080ull,
	0x4200004084010022ull,
	0x0016002080410200ull,
	0x0010400050002000ull,
	0x2040802000100088ull,
	0x0401001000210008ull,
	0x0041000500100800ull,
	0x4802808004001200ull,
	0x8004808005001200ull,
	0x0020803040800100ull,
	0x00c0248000804001ull,
	0x0010024002200050ull,
	0x0815710020010240ull,
	0x0050010010082302ull,
	0x0024050008009100ull,
	0x0820808004000200ull,
	0x0600240048102201ull,
	0x0829020000940061ull,
	0x4431806080024000ull,
	0x0490004840002000ull,
	0x1000200100410011ull,
	0x0800100080080084ull,
	0x4850080280040080ull,
	0x1000040080020080ull,
	0x1080010080800200ull,
	0x0000004200009411ull,
	0xc000400020800080ull,
	0x0440010025004080ull,
	0xaa01200082801000ull,
	0x1088800800801002ull,
	0x0010800800800400ull,
	0x0002020080800400ull,
	0x8010011004001258ull,
	0x0280800040800100ull,
	0xa820400080208000ull,
	0x0000500020004001ull,
	0x0000100020008080ull,
	0x4252000810220040ull,
	0x0208000804008080ull,
	0x40120030084e0054ull,
	0x10801001a2440018ull,
	0x8024450080460004ull,
	0x8080002000400040ull,
	0x00c0100020080020ull,
	0x064100200e10c100ull,
	0x1040102008420200ull,
	0x0002080004008280ull,
	0x4001000804000300ull,
	0x1a05000402000100ull,
	0x800080070008c080ull,
	0x0880102100408001ull,
	0x0421001080204001ull,
	0x2285410108b02001ull,
	0x0001050018a03001ull,
	0x0301000408000211ull,
	0x084a000108041002ull,
	0x0040100088420104ull,
	0x8010408409044026ull
};

extern unsigned long long const rook_magic_shift[64] = {
	12ull,
	11ull,
	11ull,
	11ull,
	11ull,
	11ull,
	11ull,
	12ull,
	11ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	11ull,
	11ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	11ull,
	11ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	11ull,
	11ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	11ull,
	11ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	11ull,
	11ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	10ull,
	11ull,
	12ull,
	11ull,
	11ull,
	11ull,
	11ull,
	11ull,
	11ull,
	12ull
};

extern unsigned long long const bishop_magic_mask[64] = {
        0x0040201008040200ull,
        0x0000402010080400ull,
        0x0000004020100a00ull,
        0x0000000040221400ull,
        0x0000000002442800ull,
        0x0000000204085000ull,
        0x0000020408102000ull,
        0x0002040810204000ull,
        0x0020100804020000ull,
        0x0040201008040000ull,
        0x00004020100a0000ull,
        0x0000004022140000ull,
        0x0000000244280000ull,
        0x0000020408500000ull,
        0x0002040810200000ull,
        0x0004081020400000ull,
        0x0010080402000200ull,
        0x0020100804000400ull,
        0x004020100a000a00ull,
        0x0000402214001400ull,
        0x0000024428002800ull,
        0x0002040850005000ull,
        0x0004081020002000ull,
        0x0008102040004000ull,
        0x0008040200020400ull,
        0x0010080400040800ull,
        0x0020100a000a1000ull,
        0x0040221400142200ull,
        0x0002442800284400ull,
        0x0004085000500800ull,
        0x0008102000201000ull,
        0x0010204000402000ull,
        0x0004020002040800ull,
        0x0008040004081000ull,
        0x00100a000a102000ull,
        0x0022140014224000ull,
        0x0044280028440200ull,
        0x0008500050080400ull,
        0x0010200020100800ull,
        0x0020400040201000ull,
        0x0002000204081000ull,
        0x0004000408102000ull,
        0x000a000a10204000ull,
        0x0014001422400000ull,
        0x0028002844020000ull,
        0x0050005008040200ull,
        0x0020002010080400ull,
        0x0040004020100800ull,
        0x0000020408102000ull,
        0x0000040810204000ull,
        0x00000a1020400000ull,
        0x0000142240000000ull,
        0x0000284402000000ull,
        0x0000500804020000ull,
        0x0000201008040200ull,
        0x0000402010080400ull,
        0x0002040810204000ull,
        0x0004081020400000ull,
        0x000a102040000000ull,
        0x0014224000000000ull,
        0x0028440200000000ull,
        0x0050080402000000ull,
        0x0020100804020000ull,
        0x0040201008040200ull
};

extern unsigned long long const bishop_magic_multiplier[64] = {
        0x8040020084009082ull,
        0x0002820204050080ull,
        0x38511102060e0000ull,
        0x0004042480000000ull,
        0xb0211040000080ccull,
        0x80020210040c8a00ull,
        0x4028820802400000ull,
        0x00228028010420d0ull,
        0x0050404941040081ull,
        0x181304100a021428ull,
        0x000408a081020002ull,
        0x0088044402800208ull,
        0x0401011040000143ull,
        0x2003209004200801ull,
        0x0000d04104602000ull,
        0x60001d0400848400ull,
        0x2804902008228800ull,
        0x1031002042008111ull,
        0x181010220421c102ull,
        0x020805208208c000ull,
        0x01210046904020d4ull,
        0x0201020210020100ull,
        0x0a12000101100200ull,
        0x0010280105180200ull,
        0x4420100020044181ull,
        0x7842020091040821ull,
        0x0202080001080022ull,
        0x0060048188008010ull,
        0x0801010010104002ull,
        0x2804050008088204ull,
        0x0010820201080210ull,
        0x40010a0011262100ull,
        0x0206181400606000ull,
        0x0404022060020400ull,
        0x0004020810010045ull,
        0x1008020080080081ull,
        0x0920040900002002ull,
        0x002000a880010040ull,
        0x0e01040100040120ull,
        0x0101020028120511ull,
        0x0012082004a10a10ull,
        0x1066380602041800ull,
        0x0001001082005004ull,
        0x1000406104002042ull,
        0x001002020a044400ull,
        0x0041200085010080ull,
        0x8004080200450404ull,
        0x100c08018a601118ull,
        0x0a048210022102c0ull,
        0x628104014c024401ull,
        0x020840540c040048ull,
        0x2001600020880000ull,
        0x8f106012020200c0ull,
        0x0000040810111208ull,
        0x4028020454042403ull,
        0x2248020802102200ull,
        0x4042840118010408ull,
        0x00820e0202410408ull,
        0x800104304a009010ull,
        0x9908840000208801ull,
        0x4001004010020888ull,
        0x424000d002100c40ull,
        0x0000201801850408ull,
        0x6440020400408b09ull
};

extern unsigned long long const bishop_magic_shift[64] = {
         6ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         6ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         7ull,
         7ull,
         7ull,
         7ull,
         5ull,
         5ull,
         5ull,
         5ull,
         7ull,
         9ull,
         9ull,
         7ull,
         5ull,
         5ull,
         5ull,
         5ull,
         7ull,
         9ull,
         9ull,
         7ull,
         5ull,
         5ull,
         5ull,
         5ull,
         7ull,
         7ull,
         7ull,
         7ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         6ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         5ull,
         6ull
};