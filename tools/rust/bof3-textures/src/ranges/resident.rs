//! Source-bound graphics byte roles; opaque bytes remain unclassified.

use super::{Entry, NontextRange, sha256_hex};

pub(super) const AREA_ANIMATION_PROFILES: &[(usize, usize, usize, u32, &str)] = &[
    (
        4,
        20_480,
        33_232,
        24,
        "c0d0cc6b90c81133c31bc5a89ac6f045aa26d9d42d805319ac8605e76ca30ec1",
    ),
    (
        4,
        26_624,
        1_604,
        20,
        "190517fb8b7b7e1944a6589bb38ce6249594e0d4671a7e7b2bf0849747c0c202",
    ),
    (
        4,
        26_624,
        3_152,
        24,
        "6d7dac7e225d177936e620560ec07460a8a3c075431d6125f992e48247b928a4",
    ),
    (
        4,
        26_624,
        4_788,
        24,
        "1221a67de057c76f05afa4a7ab87a5a69beb032444cb99bfd351b09e7090f65e",
    ),
    (
        4,
        26_624,
        10_924,
        24,
        "7ab5f515fca3d6ae85346cf2beb72b61c374e314dc73bb80fd0f45fbde8171c7",
    ),
    (
        4,
        26_624,
        12_572,
        24,
        "0a7ba2d778131ff6da5893f29f1261a7aa82b5958bc14025b9151b2c6c1d5bbc",
    ),
    (
        4,
        26_624,
        12_628,
        60,
        "3f5a3a9ad1ebde6f6ab2f46d4ea2fc54d44ce367727f117da942fe7bbc816271",
    ),
    (
        4,
        26_624,
        16_712,
        24,
        "ef567317f11b89231db7fcd1bf96e956b6ce4146203c088bb9d64e84b0527ed6",
    ),
    (
        4,
        26_624,
        24_304,
        24,
        "a7b886693e2469904f536cde88fc1491b7819afa740cdc9c10b74c9a2dfe8ca6",
    ),
    (
        4,
        26_624,
        25_572,
        24,
        "e8259db2435e39675c3a0410232e6731dd2cefc36d9bdac2630188987a7fb82f",
    ),
    (
        4,
        26_624,
        34_480,
        24,
        "e19ebf0a917c1502febc66a46eb1ff42997ae714fbd683a9cdd30d0a43abbff5",
    ),
    (
        4,
        26_624,
        38_112,
        28,
        "79bf86878665c935fb45851293e116f625e04fc841cd0f7eee608f822a699a8b",
    ),
    (
        4,
        28_672,
        8_100,
        44,
        "b490332a3298169401fdd8313d13c5f45eabbad3ef91c66b4c32400f3e31f90e",
    ),
    (
        4,
        34_816,
        10_364,
        116,
        "edb1a8d812fddc324cfe6e9aca0fe4842e671ea864b7e0200774f6cb1d0940c8",
    ),
    (
        4,
        34_816,
        25_660,
        52,
        "8e732b5af2643881e2096dd483df302abf032e2b13412baecac125146888c38b",
    ),
    (
        4,
        36_864,
        8_888,
        36,
        "6ab6050671e2f67b7f5216137bf94f1ce239672b1e045059495e210f544d55c0",
    ),
    (
        4,
        40_960,
        22_452,
        44,
        "49e15e40a0cf892b37107237e5b47397c2a027905401093c89e0ba59b052de53",
    ),
    (
        4,
        45_056,
        2_672,
        28,
        "5075d79d663f54a286ef956488f19acdd3f37231aa47fd7965a5e3180cc78b22",
    ),
    (
        4,
        47_104,
        1_604,
        20,
        "190517fb8b7b7e1944a6589bb38ce6249594e0d4671a7e7b2bf0849747c0c202",
    ),
    (
        4,
        47_104,
        2_384,
        24,
        "cc11ce365d588d5616486d180bcab44d9e2c3ab7c8de14122d90a07935cef4dc",
    ),
    (
        4,
        47_104,
        7_536,
        40,
        "4923fd9bf355b9bfe4bcea5bafce42d07de8aa95528842e8025c4aac6911f926",
    ),
    (
        4,
        47_104,
        26_500,
        36,
        "52e76d762e24bf005fba7f2d948e3829f09ca7fd9154afa768bd0f763d777465",
    ),
    (
        4,
        51_200,
        5_032,
        32,
        "b0818df71eb194ffecc695a6fd94ad5b6c8dcc784c5d1c3fd1a1d21ccf3d36b5",
    ),
    (
        4,
        51_200,
        16_624,
        88,
        "7572e0e351aa900db6bfbcecefaf7f1b5d1cac6ca4e694d7b06aeb739a8614bc",
    ),
    (
        4,
        53_248,
        31_880,
        36,
        "0fa819df97aab210583b9dc257443d0063657c92727beb496bbae0c0bd4e14bb",
    ),
    (
        4,
        53_248,
        32_048,
        56,
        "b19c87e3da7c1b6a45a0cdf4aaa278f726f8d1d9fac7de8cb926950a21b66a37",
    ),
    (
        4,
        53_248,
        35_136,
        40,
        "abc450345803dcec9e8184cbf6dc48b5751ae7b530891776053367ec711b134a",
    ),
    (
        4,
        57_344,
        3_372,
        40,
        "b022e4d37d3c5bd189e55651b48063683b3cebde727d02afb24be4e405c7be4c",
    ),
    (
        4,
        57_344,
        20_452,
        48,
        "1faa733862e59e8d3c56e23f29d6ba5c97e07c4cdb26ae9ac339f71ffb4e8028",
    ),
    (
        4,
        57_344,
        20_504,
        48,
        "d1fb19cf5e93508fc25262c69521a97e17543ee10b85610a1b9481297560658c",
    ),
    (
        4,
        67_584,
        5_408,
        40,
        "94f559688eff833caa9a594a9f9cc5c9b946212cf35f97237bc2a107024775f7",
    ),
    (
        4,
        71_680,
        1_236,
        16,
        "0896b9ee94305f1e3d19cf6de180382e7b49962d51cc5f4fee1558ae3b21e767",
    ),
    (
        4,
        71_680,
        7_752,
        76,
        "c7e13b565f2d0e42b130fe112bbfe3404c9dd2aed9bad5528808d028a43e823c",
    ),
    (
        4,
        71_680,
        39_336,
        44,
        "6c80cba8d98fc4bbb4cd64d0c30e302b0e08aae3920de19d151f52c6cc1664c1",
    ),
    (
        4,
        77_824,
        22_704,
        72,
        "de27d4b1da009928936dfd26593292e1049bd98450135117bd0fafbe4b1ade78",
    ),
    (
        4,
        86_016,
        11_852,
        40,
        "73ac8c5cf0c555205948d966a59669f53f0228af4cc10527f03fd8e40e47fa66",
    ),
    (
        4,
        86_016,
        12_592,
        28,
        "0233d9ffc09a076460937b253b4041c6491554c5ea5376c291cde2fdb584c76f",
    ),
    (
        4,
        88_064,
        34_424,
        52,
        "886b4791676d92c0f10d7e4f58639348a7218e7db876f7449ef027093e9df503",
    ),
    (
        4,
        96_256,
        36_224,
        32,
        "96f75f9a8bf04d5662fd72d20563b7d31a1f233fec63e2f68422d10cbaaad11c",
    ),
    (
        4,
        102_400,
        43_152,
        40,
        "3ef89e7bfa7f3253eb56682b38ce21fa5a576a11788a803d31b2e4438270414b",
    ),
    (
        4,
        104_448,
        10_644,
        36,
        "9f8db8a139da83511eacaf649417347c99d130965f19a5e91e6c0ab3eef8932f",
    ),
    (
        4,
        108_544,
        14_624,
        36,
        "44127e29c91635af8a59f4102b5b39496a382c6a92bf637f8da045336a6f10f0",
    ),
    (
        4,
        108_544,
        21_128,
        40,
        "798e1123ac245997cde492c8dba5970f4fb34716e88aa66da745288e7e8832c2",
    ),
    (
        4,
        108_544,
        24_404,
        44,
        "2967e205a2c944bcf9f343df57c7d33dbf9f08fe97b0d9acd311d4966ee9d6af",
    ),
    (
        4,
        114_688,
        7_688,
        64,
        "f2536bbbc266ed864133c1127b624c6a04bc60600f93cab4667986e7ba3d4adf",
    ),
    (
        4,
        114_688,
        14_128,
        100,
        "9c7f72abcd440cc7978b1b691d76c883f060ca8b3c48084c703617de32828326",
    ),
    (
        4,
        114_688,
        18_892,
        40,
        "37e08d74fc9ffe3f65387517a67f00d974df2990d5559a2966608d0d0aff3190",
    ),
    (
        4,
        118_784,
        3_620,
        52,
        "231ec09b13d36234b0c27ef57611d8cddde363f51a9d98c1ec5118b051f4fbc3",
    ),
    (
        4,
        118_784,
        39_648,
        60,
        "7e4fd4185ec9c1f74bd1b7b5b43600f89996b4c08265e1465fd2a5cde95254fc",
    ),
    (
        4,
        120_832,
        19_656,
        104,
        "598a0a0465a5a7c8356f68d3119d60d7be824bc58a1419fc50b0d5004bad3a06",
    ),
    (
        4,
        122_880,
        16_328,
        32,
        "cc7810b9694b77b1b46e6a144ff5fd004ee1bd044442d643eca13277f2cd6921",
    ),
    (
        4,
        139_264,
        36_796,
        40,
        "c4c92f27410c9cedd6d0c832d605598c3c0c1f60dc285776ad54fa30f555b205",
    ),
    (
        4,
        141_312,
        6_468,
        80,
        "7113f5fa621d90dc3005cd13e7bd78e3ccff4c062b0e141c5b4eb55bd0798543",
    ),
    (
        4,
        141_312,
        7_528,
        52,
        "0c8e76adbe9220179375f2e5e66f7f80df4ac26c84ff2a965b5c2c5d9c1c9965",
    ),
    (
        4,
        145_408,
        22_920,
        40,
        "754e2f4e6cdfb59269f8c9d9852fb94caf8e783c56259633d01c372ff57ce20f",
    ),
    (
        4,
        145_408,
        22_956,
        44,
        "cb7b09a4233f3dddba2f23ac03f96a8acfeb273aab6171b89608d28c04d17ada",
    ),
    (
        4,
        145_408,
        23_424,
        48,
        "80eedf9d192856231cec4e6a2fbf2202255754677111d5e29698c0edae9aa1c0",
    ),
    (
        4,
        147_456,
        35_352,
        92,
        "7444a77c09e2afe813b30fc3a25fd68e45b1fe5e6aee1ad56fac42d90168728e",
    ),
    (
        4,
        149_504,
        26_856,
        72,
        "ef5a7416d12feee2d0a84d6514446a3e51f4224fcf24fcff5fa25e23d309c599",
    ),
    (
        4,
        149_504,
        36_116,
        44,
        "8d2b570e693dcd2e20ea90cfa6ef94ea3625c154d10f062562beaf17a8223752",
    ),
    (
        4,
        149_504,
        42_468,
        48,
        "e00953ceb8b67ca5b8859b7910f0bbc4e24cff6fe441e1c119218fbe4d28a34f",
    ),
    (
        4,
        151_552,
        18_216,
        48,
        "228598ac8f962bba1935134ecba575bd6e9a92903708188bc98fc42f1a45b3d9",
    ),
    (
        4,
        151_552,
        20_296,
        28,
        "2b271c024e81aefcb2547ad8ebb99673bc91243fbe77360a054ba0502e2157dc",
    ),
    (
        4,
        151_552,
        40_996,
        64,
        "bc6c81a09412d9c204488854084d9aec7b9fc10a133400b1ffc175397f8b589d",
    ),
    (
        4,
        153_600,
        24_388,
        56,
        "aea2680ba47cb37a339eb6747e32a856d8822dfcd3aa8b07cf023486efc4a983",
    ),
    (
        4,
        153_600,
        25_652,
        64,
        "7128d7fa4aedd2dae4ef7e32d54f98129fe8ff1912359cc9fafb66b6f6552675",
    ),
    (
        4,
        153_600,
        30_144,
        56,
        "dbaecfdd1484d43624cae1890fb9b606d8a6484158ed6052d574c722895bec4e",
    ),
    (
        4,
        157_696,
        3_208,
        32,
        "247866f7dd82b72b624121a22ef8a731e39373132649c970f5580bdd44136161",
    ),
    (
        4,
        157_696,
        26_284,
        48,
        "7784d3e9a3ff08ee86623b381ebfd767e8bc12f4a0e5328c31def41f1cf458b5",
    ),
    (
        4,
        161_792,
        7_260,
        64,
        "af6bc2e90ac8a50e9d496eedf897806b9f01baf4e56d954bbf84cdc9dd4f0844",
    ),
    (
        4,
        169_984,
        1_604,
        20,
        "190517fb8b7b7e1944a6589bb38ce6249594e0d4671a7e7b2bf0849747c0c202",
    ),
    (
        4,
        169_984,
        29_020,
        40,
        "dbe818ed9e1759c08bd241ec291bc8d3c43297df09c80c3f208e712b8c0d8a3e",
    ),
    (
        4,
        169_984,
        37_848,
        36,
        "c133892d23636896bf32ccbde4d3445aa53ff15df2be925a31e9b7cea48a6dd3",
    ),
    (
        4,
        172_032,
        6_096,
        20,
        "421abadebca3337696f053057716ddbe7c91652af55643562d0ea6bc5f05b34f",
    ),
    (
        4,
        172_032,
        16_176,
        40,
        "c94f922d4b76a10d7a793eaf41a9f0f318ffbf1a048f25102870cbe185d296ff",
    ),
    (
        4,
        174_080,
        27_104,
        56,
        "a41dce37ac23976a590c908c2048ca918fa3d67797f98b26668b2a3e7d8aa401",
    ),
    (
        4,
        174_080,
        32_880,
        48,
        "387918488ee3a82e77d21eddf92778dc8c2b3d19a8df4cd04790150c892d05f0",
    ),
    (
        4,
        176_128,
        7_860,
        48,
        "d058d91063eacdd075ff0556ff21f21615da1bc947a77b1c3d30bcfe03ab8df2",
    ),
    (
        4,
        176_128,
        12_644,
        140,
        "e246dc1df97c88dbe04a77a5367782949767bc0a7029ab4e1a5b7853a5f5303e",
    ),
    (
        4,
        176_128,
        24_160,
        124,
        "351f40b4c41719d0617f63dd058b71e5bc20227a9f3cbb3c525005d73ddd4ac3",
    ),
    (
        4,
        176_128,
        34_052,
        52,
        "88cf0720796b6f3624f53a38f05ded7a008a229f88a98a238a14a713752815a0",
    ),
    (
        4,
        178_176,
        27_696,
        64,
        "bbedb8873b88613b61a385b2a825d66698deeb743bab6faedb3a93a7d3f466e9",
    ),
    (
        4,
        180_224,
        23_276,
        116,
        "1d94f20e8e6959793feebb2dbaaa8b66ebc46f9f7c1fb9e49b565c7987c8167f",
    ),
    (
        4,
        180_224,
        25_060,
        36,
        "a3fa50ea175882265c3caeb0b152915de9abbfb0ade2f8ffb5676bce46b43707",
    ),
    (
        4,
        182_272,
        32_032,
        44,
        "db6bd9872c6031171c765d653007f95d14b423b53c64255ea25d80495ffc0de2",
    ),
    (
        4,
        182_272,
        44_816,
        32,
        "26401af479385a7a4ddaa50a25639f657d439785f6fffabbc58a80848464b16f",
    ),
    (
        4,
        182_272,
        65_236,
        16,
        "c57264a70445e4d246761b5394a4db7cc40c875faf9067ef79588a2909edafba",
    ),
    (
        4,
        184_320,
        10_652,
        84,
        "088707a8903245d596303937ed5d081907d0d1fa8b4e74af7700651b3609af6e",
    ),
    (
        4,
        184_320,
        40_640,
        32,
        "c380e852a80a6be2446aaeb3a342af1f935968fccdb8568658cf3e0521d01be8",
    ),
    (
        4,
        186_368,
        4_008,
        48,
        "cf2918184423a493c6aec9f9368486e54d684f120b0622c70a64be426ab8c8c4",
    ),
    (
        4,
        186_368,
        6_456,
        28,
        "d2be897d971c4fd9c18f6a51699a910ea3843f6c2919c32008efb443c6b3905f",
    ),
    (
        4,
        186_368,
        52_228,
        48,
        "de5de8bd5411c991ca446c8592e2430a363d477ac5688bd0c3525b6935d4bcda",
    ),
    (
        4,
        188_416,
        35_364,
        44,
        "b16209fd487c10fd43c827fda47c5411912d2498114ef705b1d31b9ffdc0c3c1",
    ),
    (
        4,
        188_416,
        47_148,
        40,
        "248af9ee529bbafd7436fedec4bc7254b288ad3f85e59164848fb89c673bb381",
    ),
    (
        4,
        194_560,
        2_536,
        36,
        "b9646905694559458c0429f26b8d22e507e7ad9e2ead8405938a30bf981bb767",
    ),
    (
        4,
        194_560,
        3_040,
        20,
        "7b3a1ef32a5b6d318dcc2d2b728337b623d8053d4afaa6cac84b520e18ad124d",
    ),
    (
        4,
        194_560,
        19_980,
        64,
        "02bb9ed4b79106f8b3f53278cbc45b8924bd6c7bcf0f04be4863934eb4359987",
    ),
    (
        4,
        194_560,
        42_492,
        40,
        "d04ace8b80dfae6b5c91cab364f110100c9e366477da2c92646160a3e3edab91",
    ),
    (
        4,
        194_560,
        42_944,
        40,
        "b87b74c9ddbd82301a1db50d5a04831f5bf6c9b2d2feab37a59df15bcf6f107b",
    ),
    (
        4,
        196_608,
        16_816,
        84,
        "ae3c49a4227c884e9f7446cfed1f2d0d532f10c62e47987c6453ea1cf30b072a",
    ),
    (
        4,
        196_608,
        18_360,
        36,
        "d398f4d54dd3183d2e2a9964e318a5098b0e5a9e37521e6561f551dff4c1eff9",
    ),
    (
        4,
        196_608,
        21_168,
        84,
        "2e3c1365fd00ebcdae75c0014764302808d4343d96941d165782fd2744b7f36a",
    ),
    (
        4,
        196_608,
        21_576,
        76,
        "178d300134a0d4845c05519519f74aa3d440f075ddac4bdb49ceef4a115d12bb",
    ),
    (
        4,
        196_608,
        24_284,
        60,
        "98e7a5031acb048883df757679db03c14a180070ec1b14e2b0d05ffbfc4569c4",
    ),
    (
        4,
        196_608,
        32_820,
        68,
        "da95e9ed959cc57d9a9ce55bd7000ce465b28d0cbc56959d758b5c708883394a",
    ),
    (
        4,
        196_608,
        49_408,
        44,
        "e5b398976ea3bee27acda12e2fc2fd43154542e349f76f80f0d214c461e8836d",
    ),
    (
        4,
        196_608,
        57_528,
        48,
        "a6fd3aa983b403abb7452da8762c7d1348b63bdcffbcf5e62f82365526e04c29",
    ),
    (
        4,
        198_656,
        1_300,
        16,
        "7beb1b1cfa673162da15e6b523331e265579f1dba25427963e2f81094425df42",
    ),
    (
        4,
        198_656,
        1_436,
        24,
        "54ff9ab2ea889e57183172b0ac1e867905ff5112346f6f10ef5c7bcd08803e62",
    ),
    (
        4,
        198_656,
        1_604,
        20,
        "190517fb8b7b7e1944a6589bb38ce6249594e0d4671a7e7b2bf0849747c0c202",
    ),
    (
        4,
        198_656,
        1_672,
        24,
        "f936411c08069912060352c1d0979dcdb9757df866843e6c753e64d23f59bc93",
    ),
    (
        4,
        198_656,
        1_792,
        24,
        "da5b57c9ba18135e133e6190f596dd370ec16709490b5bbd4a5876e0d41e8656",
    ),
    (
        4,
        198_656,
        2_008,
        24,
        "70e2717c46d7bbf74410f12e21eda973f25447b4565d0d117c9ad2db11d95d30",
    ),
    (
        4,
        198_656,
        2_216,
        28,
        "e26ffa794571db5f4ea65c5e65d39a4888cfdcfee7e76e579b046d63626d1d7a",
    ),
    (
        4,
        198_656,
        2_624,
        28,
        "0e6b3b01b71a76071f638f0a40dd8676235d022cb441b56fcf640d6e4c017f56",
    ),
    (
        4,
        198_656,
        2_660,
        28,
        "30b183b90405439272cba965209f56f4e19816d04c502aab1a4b26daffb37c9d",
    ),
    (
        4,
        198_656,
        2_672,
        28,
        "d2314c331269c135033b7539dba395516b9076538ab20c516441d8687415dcbf",
    ),
    (
        4,
        198_656,
        2_684,
        28,
        "2ab46fc3966181da4dc26c92472219f553af557536f05e0e88ade99419947b44",
    ),
    (
        4,
        198_656,
        2_708,
        28,
        "04154522ff484e6c428d53827155c861f33bafe98e6526c4c96ba529e148df54",
    ),
    (
        4,
        198_656,
        3_064,
        28,
        "15c51c4692e3cffc119bedfce2eff5b25a7379a22d6fd178373c5c164cd22d31",
    ),
    (
        4,
        198_656,
        3_712,
        36,
        "4e6cef867e6afdfe0ba0c3fbf4d2fa9e16cf740d798fa81714e8c9e9c32a5124",
    ),
    (
        4,
        198_656,
        3_884,
        28,
        "a96ae148f58b3a7c028f571b39b4385494514bc35822682af44cf6717f9b7ae9",
    ),
    (
        4,
        198_656,
        4_348,
        40,
        "a0171d8df01e1afd39bc718ed01d342b3cde61adcb2b230e6975d9e46843e805",
    ),
    (
        4,
        198_656,
        4_840,
        24,
        "662992f762f4c7925b1d7b0a99b0cd711a3e747ae4817dc2fd3fad0bc65030b9",
    ),
    (
        4,
        198_656,
        5_856,
        20,
        "dc36ce390d9889fe59591617f67209e5fe56dfbc1edfcc00f8540f4135511de6",
    ),
    (
        4,
        198_656,
        6_040,
        36,
        "8fbeea43632a0e1b1760b11033565f021e9e7efbd1cbf6bfb31b4d12c0c68059",
    ),
    (
        4,
        198_656,
        6_488,
        40,
        "a5d9122a9064897fdbdff6641f3539f72fe50ee0566dff001f2f707394f5897d",
    ),
    (
        4,
        198_656,
        6_748,
        80,
        "68568ecc5a40fe7caa29ad649e50bdd63d30ba8f52b1675083960470677cdcb1",
    ),
    (
        4,
        198_656,
        7_368,
        96,
        "fc596976b404bbc2c1f2ab7e1b1f3ba54c95aa3a3597ad76aebc7e350189b633",
    ),
    (
        4,
        198_656,
        8_448,
        120,
        "fd833203069de37b64d71d587d7ff26652fd11fb27d7b0b640a88c1ab2d302cb",
    ),
    (
        4,
        198_656,
        8_664,
        76,
        "19b099d29035db43dba746797c132bd1f68a96bd927dff0de46f8f82c6c3c086",
    ),
    (
        4,
        198_656,
        9_948,
        52,
        "ea12802c4fe81b0507a62d3b32c92ef5533fba460ab4541fef5da284ddbf2618",
    ),
    (
        4,
        198_656,
        10_416,
        56,
        "b606fc5a8c8b52e0d64c1465f64a4bd767b2283b888871e4d67cb8ad2243c461",
    ),
    (
        4,
        198_656,
        10_556,
        52,
        "6bbee65263eb8e43ce6adb3d39ccd1e69d85023ac446d617c2a374764e94b42f",
    ),
    (
        4,
        198_656,
        10_592,
        108,
        "da67be8010dae83675d3c7e1731a236f9199cf93fb10a1d24e8c3ea8c6bfcf5f",
    ),
    (
        4,
        198_656,
        10_708,
        48,
        "d070a3476e79a9104f9357513048283e8767f77be53e826166777c3b1ecbadd9",
    ),
    (
        4,
        198_656,
        11_104,
        52,
        "4171a51723373dab664aa85e07361672d26942763d5a5f0ce608c85980679c06",
    ),
    (
        4,
        198_656,
        11_368,
        84,
        "1b36c8e96987f4c2256daa9e24e477d90c73616242be2588906525a4c8dc2c99",
    ),
    (
        4,
        198_656,
        11_968,
        72,
        "3e4732e29ef64cd14c0eb1716cc6347c98ded808cafa2c21bc5c8bc10f1e3c41",
    ),
    (
        4,
        198_656,
        12_416,
        92,
        "259746e9142f777899ae48faf59a01aebb2f802aa35b841745abee14ca4d028f",
    ),
    (
        4,
        198_656,
        12_608,
        56,
        "3ce52fcec07727104c85f5d1969b0f55bf52517f3984229a6792e3081f8bf58c",
    ),
    (
        4,
        198_656,
        13_764,
        52,
        "8f5967a01ba6146e586a94b25f1c11123abe6ef61dd894445c401e2d57bbafcd",
    ),
    (
        4,
        198_656,
        14_040,
        144,
        "8443c859322252347493962cd0b23aa24b3bee53c1e3acaff625bcb0fdc8b763",
    ),
    (
        4,
        198_656,
        14_176,
        136,
        "0f94b787870c09a6715a11455a0f30301b27766d0e421c317262652f46ede941",
    ),
    (
        4,
        198_656,
        14_328,
        140,
        "d258183ef6b7e5722a927b88deda3ff57e71b9b7717e15073d158d0b108da38d",
    ),
    (
        4,
        198_656,
        14_756,
        24,
        "febee909ea5f9cf8d32f89a0864f38894c50d1cdca677b06302dea82d90045a7",
    ),
    (
        4,
        198_656,
        18_892,
        40,
        "37e08d74fc9ffe3f65387517a67f00d974df2990d5559a2966608d0d0aff3190",
    ),
    (
        4,
        198_656,
        19_096,
        48,
        "2603982be8ed5ab15ea824753aedea4c7101045c19dff4b355f37b6cfc751011",
    ),
    (
        4,
        198_656,
        22_500,
        40,
        "03a2a6b0c4fcf73b3c12825a54ebf612bc9eeb1ec94f1d427ed1c03c628f76be",
    ),
    (
        4,
        198_656,
        27_204,
        32,
        "902ba56d95238c39889dc28fef1fa01d531b904bd3d0ff1d4b5b6ec3b311f17c",
    ),
    (
        4,
        198_656,
        27_440,
        40,
        "94a2e6cf5d73cfabb1fbfacb1b645f2751d1075ff9aded67034b744a854989ba",
    ),
    (
        4,
        198_656,
        27_672,
        36,
        "178539d6229e68fa30954152e761f7bec1ddfb272796b6ee28f69a0ef40a68d6",
    ),
    (
        4,
        198_656,
        28_068,
        68,
        "571c358fc3c97bea9ecfc1c289427288e93e900d7a3a1720c5e0b0e8d16425d6",
    ),
    (
        4,
        198_656,
        30_436,
        36,
        "ed4a6e9c8af6f66ce30fe8a313f7b02eac93ad3bbd31cd0ab943a38fbc8628c2",
    ),
    (
        4,
        198_656,
        30_584,
        44,
        "145057cb641b4f84f8b79da3cc49d5327b59df771d7aa88a817b997f5cf674d2",
    ),
    (
        4,
        198_656,
        31_608,
        32,
        "2cb9ffd90d424ff32275fe8471b07bbc5cefb77d570b43881aca0142b1e93946",
    ),
    (
        4,
        198_656,
        35_896,
        28,
        "b7d405296d2d20a17a015cbae14b1b0899377c5a2f39b4b48ac86c3906c43f6b",
    ),
    (
        4,
        198_656,
        38_276,
        36,
        "73ec0674718d7de05d11dc86a2f0745b5f72dd5185936c014de6435f56e28743",
    ),
    (
        4,
        198_656,
        38_504,
        36,
        "7941f60c1d48f2a2a090fb15db68f1ff39030b873f4777262967f5003438050d",
    ),
    (
        4,
        198_656,
        40_316,
        36,
        "941976a2ff2798a4e4857525a644cdc9b409c34a02d0790b946ad8301e095b87",
    ),
    (
        4,
        198_656,
        41_056,
        24,
        "fe2700831c1255d0b3afe6ce28de64f46676c0b2242f98ce5eaaea69e314549f",
    ),
    (
        4,
        198_656,
        44_452,
        48,
        "6e19f5555b9dbaf4ebc0656464f28ebabe20853d23d1814e64c2e5214f4d8b08",
    ),
    (
        4,
        198_656,
        49_456,
        64,
        "7b2bd1396445a96e5e0486e3a79ce85a6077c2a675ca5d7f8b0c4f29396f2cc0",
    ),
    (
        4,
        198_656,
        49_776,
        24,
        "cff7cd7ff47b5ad3b39dfcded45d2689d7394bab12b85611e0d919ed022cd8a2",
    ),
    (
        4,
        198_656,
        59_608,
        44,
        "913f5219f9006f1b3ec3144a8b0817e5783ec8e46650d3c1c42e662639dca665",
    ),
    (
        4,
        200_704,
        1_348,
        16,
        "14b0e71ebf7710fa5450216742ae7d07fe6bc33c4d0ae6f134b26a01a877946f",
    ),
    (
        4,
        200_704,
        3_152,
        36,
        "99538e541f9860b8250ec9539444efd3f60d7dc3e17a212efc4f85a80075318b",
    ),
    (
        4,
        200_704,
        8_392,
        60,
        "af1634a8b467451fbd2844cfaa4c420156755db2dacd8783b48f015214f651b8",
    ),
    (
        4,
        200_704,
        8_648,
        56,
        "bb3214ae1dbf1c7a1b942b47a3b089513020566d13d5e4b1e8988488d5e217e4",
    ),
    (
        4,
        200_704,
        8_812,
        36,
        "b2af0f3c30b93e797f8b344729991b9e400bd79079328f21f85f120b9f96faa5",
    ),
    (
        4,
        200_704,
        10_004,
        72,
        "a16b67ab082e0ee8d493b3daf0458866fa39e4da8c652f30f3a838a199ef1d5f",
    ),
    (
        4,
        200_704,
        10_524,
        60,
        "ade3f23177469da7cc175550619f56c3abaa8c5793d5f2ac61dee11c0c394728",
    ),
    (
        4,
        200_704,
        19_960,
        48,
        "d952bf3ca4c63943c66970ee798b1012ad64509cb00c2d71ca601692d8d1060e",
    ),
    (
        4,
        200_704,
        25_880,
        76,
        "756c1dfdd517130ddefd5a5041365ba9e2568985466924dd186bc1f519771d7a",
    ),
    (
        4,
        200_704,
        33_536,
        52,
        "9075bc88223699cddfb673464e048a475cf43d7b673b1f86a7ae14ad45b0c6e9",
    ),
    (
        4,
        200_704,
        36_088,
        28,
        "694c71878f97e8ea7bc53e6d2f9563f938a8b876de7027454d65e414fdde8878",
    ),
    (
        4,
        200_704,
        36_736,
        48,
        "25ce2828aba6cd8afb07112207e82ff25a0f811ad588454cb792077e9ad82418",
    ),
    (
        4,
        200_704,
        41_136,
        44,
        "82a15d3303975e00f4291df5f5f5de9d0b0ab004ff32a5b0f30f8db786ebdcd6",
    ),
    (
        4,
        200_704,
        42_428,
        52,
        "2f25c7a2bd05390269d90de291c8d5a7485d9034a21cd0feee22d56f00d8f494",
    ),
    (
        4,
        200_704,
        52_460,
        56,
        "bce692b8536c4064af3f8f0953c4b67523b5c462d4fc2fabb99e7f5110816b4c",
    ),
    (
        4,
        202_752,
        28_756,
        60,
        "020caeb4eedebd5b6c6a5bf1acb6868dffb61268b5ee42a859b88e1b9c00e688",
    ),
    (
        10,
        339_968,
        16_552,
        112,
        "1bc9c3e8eb228da398d3ccfe1d65cc02932269fd7b4bb7e3cef71086d9090331",
    ),
];

// Original BPLU349 entry0: image headers, packed words, token streams and palettes.
// Selected program/cell fields qualify separately. Mode-2 prefixes stay unresolved.
pub(super) const BPLU349_GRAPHICS_RANGES: &[(usize, usize)] = &[
    (76, 4104),
    (4184, 8),
    (4196, 899),
    (5096, 8),
    (5108, 4685),
    (9796, 4104),
    (13904, 8),
    (13916, 1677),
    (15596, 8),
    (15608, 930),
    (16540, 8),
    (16552, 6324),
    (22880, 749),
    (23632, 8),
    (23644, 828),
    (24476, 4159),
    (28636, 6844),
    (35484, 318),
    (35808, 64),
    (110668, 4104),
    (114776, 13672),
    (128452, 5146),
    (133600, 8),
    (133612, 5181),
    (138796, 10940),
    (149740, 996),
    (150740, 7549),
    (158292, 8),
    (158304, 4919),
    (163224, 8),
    (163236, 3537),
    (166776, 8),
    (166788, 5259),
    (172048, 8),
    (172060, 6015),
    (178076, 8),
    (178088, 500),
    (178596, 128),
    (221272, 5480),
    (226756, 1382),
    (228140, 8),
    (228152, 4310),
    (232464, 8),
    (232476, 931),
    (233408, 8),
    (233420, 824),
    (234248, 1599),
    (235848, 2748),
    (238600, 1567),
    (240168, 8),
    (240180, 3803),
    (243984, 6844),
    (250832, 4104),
    (254940, 8),
    (254952, 1684),
    (256640, 4245),
    (260888, 8),
    (260900, 321),
    (261228, 64),
];

pub(super) fn resident_animation_payload_ranges(
    entry: Entry,
    data: &[u8],
) -> Option<Vec<NontextRange>> {
    if entry.transfer_mode != 0 || entry.ram_ptr != 0x800d_3800 {
        return None;
    }
    let hash = sha256_hex(data);
    if !AREA_ANIMATION_PROFILES
        .iter()
        .any(|&(index, offset, length, checkword, digest)| {
            entry.index == index
                && entry.offset == offset
                && data.len() == length
                && entry.check_word == checkword
                && hash == digest
        })
    {
        return None;
    }
    let (metadata_evidence, cell_evidence) = match (entry.offset, data.len(), hash.as_str()) {
        (198_656, 11_368, "1b36c8e96987f4c2256daa9e24e477d90c73616242be2588906525a4c8dc2c99") => (
            "original US AREA024 entry 4 root/group-relative directory offsets, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact source identity; first-offset-delimited directories are not runtime selector bounds; program header byte 0 and unreferenced offset words excluded; no runtime or unique-supplier claim",
            "original US AREA024 entry 4 referenced cell counts, packed part controls, signed X/Y and unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD90; control bit7 dispatch and low-four-bit size readers 0x8014C710..0x8014C770 and 0x8014C7F0..0x8014C818; stored numeric fields only, bits4..6 and selected rectangle-helper behavior unqualified; tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
        ),
        (198_656, 10_592, "da67be8010dae83675d3c7e1731a236f9199cf93fb10a1d24e8c3ea8c6bfcf5f") => (
            "original US AREA000 entry 4 root/group-relative directory offsets, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact source identity; first-offset-delimited directories are not runtime selector bounds; program header byte 0 and unreferenced offset words excluded; no runtime or unique-supplier claim",
            "original US AREA000 entry 4 referenced cell counts, packed part controls, signed X/Y and unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD90; control bit7 dispatch and low-four-bit size readers 0x8014C710..0x8014C770 and 0x8014C7F0..0x8014C818; stored numeric fields only, bits4..6 and selected rectangle-helper behavior unqualified; tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
        ),
        (198_656, 11_968, "3e4732e29ef64cd14c0eb1716cc6347c98ded808cafa2c21bc5c8bc10f1e3c41") => (
            "original US AREA001 entry 4 root/group-relative directory offsets, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact source identity; first-offset-delimited directories are not runtime selector bounds; program header byte 0 and unreferenced offset words excluded; no runtime or unique-supplier claim",
            "original US AREA001 entry 4 referenced cell counts, packed part controls, signed X/Y and unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD90; control bit7 dispatch and low-four-bit size readers 0x8014C710..0x8014C770 and 0x8014C7F0..0x8014C818; stored numeric fields only, bits4..6 and selected rectangle-helper behavior unqualified; tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
        ),
        (198_656, 11_104, "4171a51723373dab664aa85e07361672d26942763d5a5f0ce608c85980679c06") => (
            "original US AREA002 entry 4 root/group-relative directory offsets, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact source identity; first-offset-delimited directories are not runtime selector bounds; program header byte 0 and unreferenced offset words excluded; no runtime or unique-supplier claim",
            "original US AREA002 entry 4 referenced cell counts, packed part controls, signed X/Y and unsigned U/V fields; pointer chain 0x8014D964, 0x8014DBFC..0x8014DC04 and 0x8014C6D8..0x8014C6F4; count/five-byte part consumer 0x8014C6F8..0x8014CD90; control bit7 dispatch and low-four-bit size readers 0x8014C710..0x8014C770 and 0x8014C7F0..0x8014C818; stored numeric fields only, bits4..6 and selected rectangle-helper behavior unqualified; tails and unreferenced cells excluded; exact source identity; no runtime selection or rendering claim",
        ),
        (339_968, 16_552, "1bc9c3e8eb228da398d3ccfe1d65cc02932269fd7b4bb7e3cef71086d9090331") => (
            "original US identical AREA030/AREA089/AREA129 entry 10 resident profile; root/group-relative directories, program counts, duration/operand pairs and ordinary referenced cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; exact original archive and descriptor/payload tuple with conditional world-slot/raw-loader/startup-root binding; checked finite extents and one direct control-target fetch; only program530 token14 operand14 permits lookup590..592 to zero-part target563, with lookup bytes independently typed as ordinary cell fields and target as metadata; no alias-specific range emitted; all other aliases, program header byte0, unreferenced fields, gaps and tails excluded; stable code, source, work and pointers, ordinary helper returns and noninterference remain conditions; no actual invocation, residency, selector bounds, rendering, text absence, unique supplier or safe-edit claim",
            "original US identical AREA030/AREA089/AREA129 entry 10 resident profile; ordinary referenced cell counts, packed controls, signed X/Y and unsigned U/V in checked five-byte parts; original pointer construction 0x8014C6D8..0x8014C6F4 and consumer 0x8014C6F8..0x8014CD90, bit7/low-four-bit numeric readers; exact original archive and descriptor/payload tuple with conditional permitted completed raw transfer and stable startup base/work/pointers; exact zero-cell alias emits no additional bytes or records and requires independently typed lookup and target bytes; bits4..6, selected helper behavior, unreferenced cells and allocation tails unqualified; ordinary helper returns and noninterference remain conditions; no actual selection, transfer, residency, rendering, ownership, resource safety or editing claim",
        ),
        _ => (
            "original US exact AREA resident profile; root/group-relative directories, program counts, duration/operand pairs and referenced relative cell offsets; original consumers 0x8014D878..0x8014D8AC, 0x8014D928..0x8014D964 and 0x8014DBB4..0x8014DC04; world-index file-slot and physical archive binding, permitted completed raw loading and stable startup-published base 0x800D3800 condition source applicability; exact original archive and descriptor/payload tuple; checked finite first-offset directory, program, direct/control-target fetch and cell extents; first offsets do not establish runtime selector bounds; program header byte 0, metadata aliases, unreferenced lookup words, cells, gaps and tails excluded; no actual invocation, transfer, residency, complete program consumption, rendering, unique supplier or safe-edit claim",
            "original US exact AREA resident profile; referenced cell counts, packed controls, signed X/Y and unsigned U/V in bounded five-byte parts; original pointer construction 0x8014C6D8..0x8014C6F4 and consumer 0x8014C6F8..0x8014CD90, control bit7 and low-four-bit numeric readers; exact original archive and descriptor/payload tuple with conditional file-slot/raw-loader/startup-root binding; all declared cell extents checked, referenced numeric fields only and overlapping roles refused; bits4..6, selected rectangle helper, unreferenced cells and allocation tails unqualified; no actual selection, transfer, residency, rendering, text absence, ownership, resource safety or safe-edit claim",
        ),
    };
    resident_animation_field_ranges(data, metadata_evidence, cell_evidence, entry.index == 10)
}

pub(super) fn resident_animation_field_ranges(
    data: &[u8],
    metadata_evidence: &'static str,
    cell_evidence: &'static str,
    area030_alias: bool,
) -> Option<Vec<NontextRange>> {
    if data.len() > 1024 * 1024 {
        return None;
    }
    let root_end = usize::try_from(u32::from_le_bytes(data.get(..4)?.try_into().ok()?)).ok()?;
    if root_end == 0 || root_end % 4 != 0 || root_end / 4 > 65_536 {
        return None;
    }
    let mut groups = data
        .get(..root_end)?
        .as_chunks::<4>()
        .0
        .iter()
        .map(|word| usize::try_from(u32::from_le_bytes(*word)).ok())
        .collect::<Option<Vec<_>>>()?;
    if groups
        .iter()
        .any(|&group| group < root_end || group >= data.len())
    {
        return None;
    }
    groups.sort_unstable();
    groups.dedup();
    let mut metadata = vec![(0, root_end)];
    let mut referenced_cells = std::collections::BTreeMap::new();
    let mut alias_reads = Vec::new();
    for (group_index, &group) in groups.iter().enumerate() {
        let group_end = groups.get(group_index + 1).copied().unwrap_or(data.len());
        let first = usize::from(u16::from_le_bytes(
            data.get(group..group.checked_add(2)?)?.try_into().ok()?,
        ));
        let directory_end = group.checked_add(first)?;
        if first == 0 || first % 2 != 0 || first / 2 > 256 || directory_end > group_end {
            return None;
        }
        metadata.push((group, directory_end));
        let mut programs = data
            .get(group..directory_end)?
            .as_chunks::<2>()
            .0
            .iter()
            .map(|word| group.checked_add(usize::from(u16::from_le_bytes(*word))))
            .collect::<Option<Vec<_>>>()?;
        if programs
            .iter()
            .any(|&program| program < directory_end || program >= group_end)
        {
            return None;
        }
        programs.sort_unstable();
        programs.dedup();
        for (program_index, &program) in programs.iter().enumerate() {
            let program_end = programs
                .get(program_index + 1)
                .copied()
                .unwrap_or(group_end);
            let header_end = program.checked_add(2)?;
            let header = data.get(program..header_end)?;
            let cell_count = usize::from(header[0]);
            let step_count = usize::from(header[1]);
            if cell_count == 0 || step_count == 0 {
                return None;
            }
            let cell_table = header_end.checked_add(step_count.checked_mul(2)?)?;
            let table_end = cell_table.checked_add(cell_count.checked_mul(2)?)?;
            if table_end > program_end {
                return None;
            }
            let steps = data.get(header_end..cell_table)?;
            let offsets = data
                .get(cell_table..table_end)?
                .as_chunks::<2>()
                .0
                .iter()
                .map(|word| cell_table.checked_add(usize::from(u16::from_le_bytes(*word))))
                .collect::<Option<Vec<_>>>()?;
            if offsets.first().copied()? != table_end
                || offsets
                    .iter()
                    .any(|&cell| cell < table_end || cell >= program_end)
            {
                return None;
            }
            let mut distinct_cells = offsets.clone();
            distinct_cells.sort_unstable();
            distinct_cells.dedup();
            for (index, &cell) in distinct_cells.iter().enumerate() {
                let allocation_end = distinct_cells
                    .get(index + 1)
                    .copied()
                    .unwrap_or(program_end);
                let field_end = cell
                    .checked_add(1)?
                    .checked_add(usize::from(*data.get(cell)?).checked_mul(5)?)?;
                if field_end > allocation_end {
                    return None;
                }
            }
            let mut selected = vec![false; cell_count];
            let mut select = |token_index: usize| -> Option<()> {
                let token_offset = token_index.checked_mul(2)?;
                let token = steps.get(token_offset..token_offset.checked_add(2)?)?;
                let operand = usize::from(token[1]);
                if let Some(cell) = selected.get_mut(operand) {
                    *cell = true;
                    return Some(());
                }
                if !area030_alias
                    || program != 530
                    || program_end != 772
                    || cell_count != 14
                    || step_count != 15
                    || cell_table != 562
                    || token_index != 14
                    || token != [2, 14]
                {
                    return None;
                }
                let lookup = cell_table.checked_add(operand.checked_mul(2)?)?;
                let relative = usize::from(u16::from_le_bytes(
                    data.get(lookup..lookup.checked_add(2)?)?.try_into().ok()?,
                ));
                let target = cell_table.checked_add(relative)?;
                if lookup != 590 || target != 563 || data.get(target) != Some(&0) {
                    return None;
                }
                alias_reads.push((lookup, target));
                Some(())
            };
            select(0)?;
            for (token_index, token) in steps.as_chunks::<2>().0.iter().enumerate() {
                if token[0] < 0x80 {
                    select(token_index)?;
                } else {
                    let target = usize::from(token[1]);
                    if target >= step_count {
                        return None;
                    }
                    select(target)?;
                }
            }
            metadata.push((program.checked_add(1)?, cell_table));
            for (index, &cell) in offsets.iter().enumerate() {
                if !selected[index] {
                    continue;
                }
                let lookup = cell_table.checked_add(index.checked_mul(2)?)?;
                metadata.push((lookup, lookup.checked_add(2)?));
                let count = usize::from(*data.get(cell)?);
                let allocation_end = distinct_cells
                    .get(distinct_cells.binary_search(&cell).ok()?.checked_add(1)?)
                    .copied()
                    .unwrap_or(program_end);
                if let Some(previous) = referenced_cells.insert(cell, (count, allocation_end))
                    && previous != (count, allocation_end)
                {
                    return None;
                }
            }
        }
    }
    let mut roles = vec![0u8; data.len()];
    for (start, end) in metadata {
        roles.get_mut(start..end)?.fill(1);
    }
    let mut ranges = Vec::new();
    let mut cursor = 0;
    while cursor < roles.len() {
        if roles[cursor] != 1 {
            cursor += 1;
            continue;
        }
        let start = cursor;
        while cursor < roles.len() && roles[cursor] == 1 {
            cursor += 1;
        }
        ranges.push(NontextRange {
            offset: start,
            length: cursor - start,
            evidence: metadata_evidence,
        });
    }
    for (cell, (count, _)) in referenced_cells {
        let end = cell.checked_add(1)?.checked_add(count.checked_mul(5)?)?;
        let occupied = roles.get_mut(cell..end)?;
        if occupied.iter().any(|&role| role != 0) {
            return None;
        }
        occupied.fill(2);
        ranges.push(NontextRange {
            offset: cell,
            length: 1,
            evidence: cell_evidence,
        });
        for part in 0..count {
            ranges.push(NontextRange {
                offset: cell + 1 + 5 * part,
                length: 5,
                evidence: cell_evidence,
            });
        }
    }
    for (lookup, target) in alias_reads {
        if roles
            .get(lookup..lookup.checked_add(2)?)?
            .iter()
            .any(|&role| role != 2)
            || roles.get(target) != Some(&1)
        {
            return None;
        }
    }
    ranges.sort_by_key(|range| range.offset);
    Some(ranges)
}

pub(super) fn area_surface_field_ranges(data: &[u8], check_word: u32) -> Option<Vec<NontextRange>> {
    let header = data.get(..22)?;
    if data.len() > 1024 * 1024
        || header[0] == 0
        || header[1] == 0
        || u32::from_le_bytes(header[..4].try_into().ok()?) != check_word
    {
        return None;
    }
    let half = |offset: usize| -> Option<usize> {
        Some(usize::from(u16::from_le_bytes(
            data.get(offset..offset.checked_add(2)?)?.try_into().ok()?,
        )))
    };
    let cells = usize::from(header[0]).checked_mul(usize::from(header[1]))?;
    let grid_offset = half(2)?.checked_mul(4)?;
    let grid_end = grid_offset.checked_add(cells.checked_mul(2)?)?;
    let grid = data.get(grid_offset..grid_end)?;
    let words_offset = grid_offset.checked_add(cells.div_ceil(2).checked_mul(4)?)?;
    let byte_grid = half(20)?.checked_mul(4)?;
    let byte_grid_end = byte_grid.checked_add(cells)?;
    data.get(byte_grid..byte_grid_end)?;

    let mut spans = vec![(0, 4), (20, 22), (grid_offset, grid_end)];
    let mut ids: Vec<_> = grid
        .as_chunks::<2>()
        .0
        .iter()
        .map(|pair| u16::from_le_bytes([pair[0], pair[1]]))
        .collect();
    ids.sort_unstable();
    ids.dedup();
    for id in ids {
        let id = usize::from(id);
        if id == 0 {
            continue;
        }
        let offset = words_offset.checked_add(id.checked_mul(4)?)?;
        let end = offset.checked_add(4)?;
        let word = u32::from_le_bytes(data.get(offset..end)?.try_into().ok()?);
        spans.push((offset, end));
        if word & 0xf00 == 0 {
            continue;
        }
        let paired = word & 0x800 != 0;
        let size = if paired { 8 } else { 4 };
        let field = (if paired { 6 } else { 4 }) + 4 * ((word >> 28) & 3) as usize;
        let index = (word & if paired { 0x7ff } else { 0xff }) as usize;
        let coordinates = half(field)?
            .checked_mul(4)?
            .checked_add(index.checked_mul(size)?)?;
        let end = coordinates.checked_add(size)?;
        data.get(coordinates..end)?;
        spans.push((field, field + 2));
        spans.push((coordinates, end));
    }
    spans.sort_unstable();
    let mut merged: Vec<(usize, usize)> = Vec::new();
    for (start, end) in spans {
        match merged.last_mut() {
            Some(previous) if start <= previous.1 => previous.1 = previous.1.max(end),
            _ => merged.push((start, end)),
        }
    }
    let evidence = "original US raw area surface dimensions, relative offsets, declared u16 cell IDs, nonzero-ID primary packed words and selected coordinate reads; original consumers 0x80154294..0x80154438, 0x80155560..0x801556FC and byte-grid pointer 0x80154094..0x801540F8; exact archive identity, checkword agreement and checked source bounds; unused headers, byte-grid contents, conditional sequential words, gaps and tails excluded; no natural visitation, full allocation, rendering or editing claim";
    let mut ranges: Vec<_> = merged
        .iter()
        .map(|&(offset, end)| NontextRange {
            offset,
            length: end - offset,
            evidence,
        })
        .collect();
    let evidence = "original US raw area numeric u8 grid; header-relative offset at bytes 20..22 multiplied by four and published to 0x8014931C by original SLUS 0x80154094..0x801540F8; complete original getter 0x80166CB0..0x80166CEC returns unchanged byte at base+y*width+x and original GAME caller applies numeric masks/comparisons; finite declared-grid coordinates cover every width*height byte with signed16-safe inputs; exact original archive, raw mode, load, checkword agreement and checked source bounds; prior fields retain their own evidence; no global caller-coordinate bounds, natural visitation, terrain semantics, full allocation, rendering or editing claim";
    let mut numeric_planes = vec![(byte_grid, byte_grid_end, evidence)];
    if let Some(offset) = half(32) {
        numeric_planes.push((32, 34, "original US raw area unsigned-u16 relative-offset field at payload bytes 32..34; original initializer 0x80157F98..0x801580A4 reads it with LHU 0x80157FA4, scales by four and adds payload base 0x80104000 before the first list-word read; exact original archive, raw mode, load, checkword agreement and checked two-byte source extent; prior fields retain their own evidence; list storage and nonzero callbacks require separate qualification; no actual invocation, callback semantics, auxiliary storage, resource safety, rendering or editing claim"));
        let terminator = offset.checked_mul(4).and_then(|start| {
            let end = start.checked_add(4)?;
            (data.get(start..end)? == [0, 0, 0, 0]).then_some((start, end))
        });
        if let Some((start, end)) = terminator {
            numeric_planes.push((start, end, "original US raw area first callback-list zero terminator; original initializer 0x80157F98..0x801580A4 uses the unsigned-u16 offset at payload bytes 32..34, scaled by four, for LW 0x80157FD4 and zero-test 0x80157FDC; complete four-byte source extent is checked and must contain u32 zero; exact original archive, raw mode, load and checkword agreement; prior fields retain their own evidence; nonzero records, subsequent list words, callback targets and auxiliary storage excluded; no actual invocation, initialization, ownership, callback semantics, resource safety, rendering or editing claim"));
        }
    }
    if let Some(offset) = half(42) {
        numeric_planes.push((42, 44, "original US raw area unsigned-u16 relative-offset field at payload bytes 42..44; original sampler 0x80154ABC..0x80154ED0 and complete setter 0x80155038..0x8015507C scale the field by four and add payload base 0x80104000 before numeric signed-byte reads or writes; exact original archive, raw mode, load, checkword agreement and checked two-byte source extent; prior fields retain their own evidence; auxiliary storage, initialization, ownership, coordinate safety, terrain semantics, full allocation, rendering and editing excluded"));
        if let Some((start, end)) = offset.checked_mul(4).and_then(|start| {
            let end = start.checked_add(cells)?;
            data.get(start..end)?;
            Some((start, end))
        }) {
            numeric_planes.push((start, end, "original US raw area stored signed-byte auxiliary plane; complete original sampler 0x80154ABC..0x80154ED0 scales the unsigned-u16 offset at payload bytes 42..44 by four and reads base+y*width+x through LB 0x80154B84 for integral coordinates, then multiplies the signed byte numerically; complete setter 0x80155038..0x8015507C uses the same address for SB; finite declared-grid coordinates cover every width*height byte with signed16-safe inputs; exact original archive, raw mode, load, checkword agreement and complete checked in-payload extent; prior fields retain their own evidence; no partial prefix, external storage, initialization, global caller bounds, fractional neighbor safety, natural visitation, terrain semantics, rendering or editing claim"));
        }
    }
    if let Some(end) = cells
        .checked_mul(4)
        .and_then(|bytes| 48usize.checked_add(bytes))
        .filter(|&end| data.get(48..end).is_some())
    {
        numeric_planes.push((48, end, "original US raw area packed-u32 numeric plane; complete original sampler 0x80154ABC..0x80154ED0 constructs base 0x80104030 and reads one whole word at base+4*(y*width+x) for integral coordinates through LW 0x80154B98; finite declared-grid coordinates cover every width*height word with signed16-safe inputs; exact original archive, raw mode, load, checkword agreement and complete checked extent; prior fields retain their own evidence; separate auxiliary signed-byte plane excluded; no partial prefix, global caller bounds, neighbor safety, natural visitation, terrain semantics, full allocation, rendering or editing claim"));
    }
    for (start, end, evidence) in numeric_planes {
        ranges.sort_by_key(|range| range.offset);
        let mut additions = Vec::new();
        let mut cursor = start;
        for range in &ranges {
            if cursor >= end {
                break;
            }
            let known_end = range.offset + range.length;
            if known_end <= cursor {
                continue;
            }
            if cursor < range.offset {
                additions.push(NontextRange {
                    offset: cursor,
                    length: range.offset.min(end) - cursor,
                    evidence,
                });
            }
            cursor = cursor.max(known_end);
        }
        if cursor < end {
            additions.push(NontextRange {
                offset: cursor,
                length: end - cursor,
                evidence,
            });
        }
        ranges.extend(additions);
    }
    ranges.sort_by_key(|range| range.offset);
    Some(ranges)
}
