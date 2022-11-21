#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600

#include <windows.h>
#include <unordered_map>
#include <xinput.h>

#include "PatchUtils.h"
#include "Mutex.h"

#include "redelbe.h"
#include "rdbemu.h"
#include "layer2.h"
#include "game.h"
#include "ui.h"
#include "misc.h"
#include "debug.h"

static const std::unordered_map<uint32_t, std::string> hash_to_costume =
{
	{ 0xa81ccb23, "CMN_COS_000" },
	{ 0xa81ee4ea, "KAS_COS_000" },
	{ 0xafd08a49, "KAS_COS_001" },
	{ 0xb7822fa8, "KAS_COS_002" },
	{ 0xbf33d507, "KAS_COS_003" },
	{ 0xc6e57a66, "KAS_COS_004" },
	{ 0xce971fc5, "KAS_COS_005" },
	{ 0xd648c524, "KAS_COS_006" },
	{ 0xddfa6a83, "KAS_COS_007" },
	{ 0xe5ac0fe2, "KAS_COS_008" },
	{ 0xed5db541, "KAS_COS_009" },
	{ 0x3d0397ab, "KAS_COS_010" },
	{ 0x44b53d0a, "KAS_COS_011" },
	{ 0xd999efcb, "KAS_COS_021" },
	{ 0xe14b952a, "KAS_COS_022" },
	{ 0x66ccfd2d, "KAS_COS_030" },
	{ 0x6e7ea28c, "KAS_COS_031" },
	{ 0x763047eb, "KAS_COS_032" },
	{ 0x7de1ed4a, "KAS_COS_033" },
	{ 0x859392a9, "KAS_COS_034" },
	{ 0x8d453808, "KAS_COS_035" },
	{ 0x94f6dd67, "KAS_COS_036" },
	{ 0x9ca882c6, "KAS_COS_037" },
	{ 0x9c685609, "KAS_COS_100" },
	{ 0xa419fb68, "KAS_COS_101" },
	{ 0xabcba0c7, "KAS_COS_102" },
	{ 0xb37d4626, "KAS_COS_103" },
	{ 0xbb2eeb85, "KAS_COS_104" },
	{ 0xc2e090e4, "KAS_COS_105" },
	{ 0xca923643, "KAS_COS_106" },
	{ 0xd243dba2, "KAS_COS_107" },
	{ 0xa81ee3d3, "BAS_COS_000" },
	{ 0xafd08932, "BAS_COS_001" },
	{ 0xc6e5794f, "BAS_COS_004" },
	{ 0xddfa696c, "BAS_COS_007" },
	{ 0x3d039694, "BAS_COS_010" },
	{ 0x44b53bf3, "BAS_COS_011" },
	{ 0x66ccfc16, "BAS_COS_030" },
	{ 0x763046d4, "BAS_COS_032" },
	{ 0xac0bcc6d, "BAS_COS_039" },
	{ 0xd243da8b, "BAS_COS_107" },
	{ 0x206c57ed, "BAS_COS_107b" },
	{ 0xa8219e0d, "BAY_COS_000" },
	{ 0xafd3436c, "BAY_COS_001" },
	{ 0xc6e83389, "BAY_COS_004" },
	{ 0xddfd23a6, "BAY_COS_007" },
	{ 0x3d0650ce, "BAY_COS_010" },
	{ 0x44b7f62d, "BAY_COS_011" },
	{ 0x66cfb650, "BAY_COS_030" },
	{ 0x7633010e, "BAY_COS_032" },
	{ 0xd24694c5, "BAY_COS_107" },
	{ 0xa81c9ef0, "JAN_COS_000" },
	{ 0xafce444f, "JAN_COS_001" },
	{ 0xc6e3346c, "JAN_COS_004" },
	{ 0xddf82489, "JAN_COS_007" },
	{ 0x3d0151b1, "JAN_COS_010" },
	{ 0x44b2f710, "JAN_COS_011" },
	{ 0x66cab733, "JAN_COS_030" },
	{ 0x762e01f1, "JAN_COS_032" },
	{ 0xd24195a8, "JAN_COS_107" },
	{ 0xa81a6857, "LEI_COS_000" },
	{ 0xafcc0db6, "LEI_COS_001" },
	{ 0xb77db315, "LEI_COS_002" },
	{ 0xbf2f5874, "LEI_COS_003" },
	{ 0xc6e0fdd3, "LEI_COS_004" },
	{ 0xce92a332, "LEI_COS_005" },
	{ 0xddf5edf0, "LEI_COS_007" },
	{ 0x3cff1b18, "LEI_COS_010" },
	{ 0x44b0c077, "LEI_COS_011" },
	{ 0x66c8809a, "LEI_COS_030" },
	{ 0x6e7a25f9, "LEI_COS_031" },
	{ 0x762bcb58, "LEI_COS_032" },
	{ 0x7ddd70b7, "LEI_COS_033" },
	{ 0x858f1616, "LEI_COS_034" },
	{ 0x8d40bb75, "LEI_COS_035" },
	{ 0x94f260d4, "LEI_COS_036" },
	{ 0x9ca40633, "LEI_COS_037" },
	{ 0x9c63d976, "LEI_COS_100" },
	{ 0xa4157ed5, "LEI_COS_101" },
	{ 0xabc72434, "LEI_COS_102" },
	{ 0xb378c993, "LEI_COS_103" },
	{ 0xbb2a6ef2, "LEI_COS_104" },
	{ 0xc2dc1451, "LEI_COS_105" },
	{ 0xca8db9b0, "LEI_COS_106" },
	{ 0xd23f5f0f, "LEI_COS_107" },
	{ 0xa81c71a6, "HTM_COS_000" },
	{ 0xafce1705, "HTM_COS_001" },
	{ 0xb77fbc64, "HTM_COS_002" },
	{ 0xbf3161c3, "HTM_COS_003" },
	{ 0xc6e30722, "HTM_COS_004" },
	{ 0xce94ac81, "HTM_COS_005" },
	{ 0xddf7f73f, "HTM_COS_007" },
	{ 0x3d012467, "HTM_COS_010" },
	{ 0x44b2c9c6, "HTM_COS_011" },
	{ 0x66ca89e9, "HTM_COS_030" },
	{ 0x6e7c2f48, "HTM_COS_031" },
	{ 0x762dd4a7, "HTM_COS_032" },
	{ 0x7ddf7a06, "HTM_COS_033" },
	{ 0x85911f65, "HTM_COS_034" },
	{ 0x8d42c4c4, "HTM_COS_035" },
	{ 0x94f46a23, "HTM_COS_036" },
	{ 0x9ca60f82, "HTM_COS_037" },
	{ 0xa4178824, "HTM_COS_101" },
	{ 0xabc92d83, "HTM_COS_102" },
	{ 0xb37ad2e2, "HTM_COS_103" },
	{ 0xbb2c7841, "HTM_COS_104" },
	{ 0xc2de1da0, "HTM_COS_105" },
	{ 0xca8fc2ff, "HTM_COS_106" },
	{ 0xd241685e, "HTM_COS_107" },
	{ 0xa8202899, "RYU_COS_000" },
	{ 0xafd1cdf8, "RYU_COS_001" },
	{ 0xc6e6be15, "RYU_COS_004" },
	{ 0xddfbae32, "RYU_COS_007" },
	{ 0xe5ad5391, "RYU_COS_008" },
	{ 0x3d04db5a, "RYU_COS_010" },
	{ 0x44b680b9, "RYU_COS_011" },
	{ 0x66ce40dc, "RYU_COS_030" },
	{ 0x76318b9a, "RYU_COS_032" },
	{ 0x9c6999b8, "RYU_COS_100" },
	{ 0xd2451f51, "RYU_COS_107" },
	{ 0xa81fb304, "HYT_COS_000" },
	{ 0xafd15863, "HYT_COS_001" },
	{ 0xc6e64880, "HYT_COS_004" },
	{ 0xddfb389d, "HYT_COS_007" },
	{ 0x3d0465c5, "HYT_COS_010" },
	{ 0x44b60b24, "HYT_COS_011" },
	{ 0x66cdcb47, "HYT_COS_030" },
	{ 0x76311605, "HYT_COS_032" },
	{ 0x9c692423, "HYT_COS_100" },
	{ 0xd244a9bc, "HYT_COS_107" },
	{ 0xa81e70c9, "MAR_COS_000" },
	{ 0xafd01628, "MAR_COS_001" },
	{ 0xb781bb87, "MAR_COS_002" },
	{ 0xbf3360e6, "MAR_COS_003" },
	{ 0xc6e50645, "MAR_COS_004" },
	{ 0xce96aba4, "MAR_COS_005" },
	{ 0xddf9f662, "MAR_COS_007" },
	{ 0x3d03238a, "MAR_COS_010" },
	{ 0x44b4c8e9, "MAR_COS_011" },
	{ 0xd1e7d64b, "MAR_COS_020" },
	{ 0xe14b2109, "MAR_COS_022" },
	{ 0x66cc890c, "MAR_COS_030" },
	{ 0x6e7e2e6b, "MAR_COS_031" },
	{ 0x762fd3ca, "MAR_COS_032" },
	{ 0x7de17929, "MAR_COS_033" },
	{ 0x85931e88, "MAR_COS_034" },
	{ 0x8d44c3e7, "MAR_COS_035" },
	{ 0x94f66946, "MAR_COS_036" },
	{ 0x9ca80ea5, "MAR_COS_037" },
	{ 0xa459b404, "MAR_COS_038" },
	{ 0xac0b5963, "MAR_COS_039" },
	{ 0x9c67e1e8, "MAR_COS_100" },
	{ 0xa4198747, "MAR_COS_101" },
	{ 0xabcb2ca6, "MAR_COS_102" },
	{ 0xb37cd205, "MAR_COS_103" },
	{ 0xbb2e7764, "MAR_COS_104" },
	{ 0xc2e01cc3, "MAR_COS_105" },
	{ 0xca91c222, "MAR_COS_106" },
	{ 0xd2436781, "MAR_COS_107" },
	{ 0xa817bd5f, "NIC_COS_000" },
	{ 0xc6de52db, "NIC_COS_004" },
	{ 0xce8ff83a, "NIC_COS_005" },
	{ 0xd6419d99, "NIC_COS_006" },
	{ 0xddf342f8, "NIC_COS_007" },
	{ 0xe5a4e857, "NIC_COS_008" },
	{ 0x3cfc7020, "NIC_COS_010" },
	{ 0x44ae157f, "NIC_COS_011" },
	{ 0x66c5d5a2, "NIC_COS_030" },
	{ 0x6e777b01, "NIC_COS_031" },
	{ 0x76292060, "NIC_COS_032" },
	{ 0x7ddac5bf, "NIC_COS_033" },
	{ 0x858c6b1e, "NIC_COS_034" },
	{ 0x8d3e107d, "NIC_COS_035" },
	{ 0x94efb5dc, "NIC_COS_036" },
	{ 0xa453009a, "NIC_COS_038" },
	{ 0xac04a5f9, "NIC_COS_039" },
	{ 0xa412d3dd, "NIC_COS_101" },
	{ 0xabc4793c, "NIC_COS_102" },
	{ 0xb3761e9b, "NIC_COS_103" },
	{ 0xbb27c3fa, "NIC_COS_104" },
	{ 0xc2d96959, "NIC_COS_105" },
	{ 0xca8b0eb8, "NIC_COS_106" },
	{ 0xd23cb417, "NIC_COS_107" },
	{ 0xa81b7680, "KOK_COS_000" },
	{ 0xafcd1bdf, "KOK_COS_001" },
	{ 0xb77ec13e, "KOK_COS_002" },
	{ 0xbf30669d, "KOK_COS_003" },
	{ 0xc6e20bfc, "KOK_COS_004" },
	{ 0xce93b15b, "KOK_COS_005" },
	{ 0xddf6fc19, "KOK_COS_007" },
	{ 0x3d002941, "KOK_COS_010" },
	{ 0x44b1cea0, "KOK_COS_011" },
	{ 0x66c98ec3, "KOK_COS_030" },
	{ 0x6e7b3422, "KOK_COS_031" },
	{ 0x762cd981, "KOK_COS_032" },
	{ 0x7dde7ee0, "KOK_COS_033" },
	{ 0x8590243f, "KOK_COS_034" },
	{ 0x8d41c99e, "KOK_COS_035" },
	{ 0x94f36efd, "KOK_COS_036" },
	{ 0x9ca5145c, "KOK_COS_037" },
	{ 0xa4168cfe, "KOK_COS_101" },
	{ 0xabc8325d, "KOK_COS_102" },
	{ 0xb379d7bc, "KOK_COS_103" },
	{ 0xbb2b7d1b, "KOK_COS_104" },
	{ 0xc2dd227a, "KOK_COS_105" },
	{ 0xca8ec7d9, "KOK_COS_106" },
	{ 0xd2406d38, "KOK_COS_107" },
	{ 0xa81d6de3, "NYO_COS_000" },
	{ 0xafcf1342, "NYO_COS_001" },
	{ 0xb780b8a1, "NYO_COS_002" },
	{ 0xbf325e00, "NYO_COS_003" },
	{ 0xc6e4035f, "NYO_COS_004" },
	{ 0xce95a8be, "NYO_COS_005" },
	{ 0xddf8f37c, "NYO_COS_007" },
	{ 0x3d0220a4, "NYO_COS_010" },
	{ 0x44b3c603, "NYO_COS_011" },
	{ 0x66cb8626, "NYO_COS_030" },
	{ 0x6e7d2b85, "NYO_COS_031" },
	{ 0x762ed0e4, "NYO_COS_032" },
	{ 0x7de07643, "NYO_COS_033" },
	{ 0x85921ba2, "NYO_COS_034" },
	{ 0x8d43c101, "NYO_COS_035" },
	{ 0x94f56660, "NYO_COS_036" },
	{ 0x9ca70bbf, "NYO_COS_037" },
	{ 0xa4188461, "NYO_COS_101" },
	{ 0xabca29c0, "NYO_COS_102" },
	{ 0xb37bcf1f, "NYO_COS_103" },
	{ 0xbb2d747e, "NYO_COS_104" },
	{ 0xc2df19dd, "NYO_COS_105" },
	{ 0xca90bf3c, "NYO_COS_106" },
	{ 0xd242649b, "NYO_COS_107" },
	{ 0xa818323a, "RID_COS_000" },
	{ 0xafc9d799, "RID_COS_001" },
	{ 0xc6dec7b6, "RID_COS_004" },
	{ 0xddf3b7d3, "RID_COS_007" },
	{ 0x3cfce4fb, "RID_COS_010" },
	{ 0x44ae8a5a, "RID_COS_011" },
	{ 0x66c64a7d, "RID_COS_030" },
	{ 0x7629953b, "RID_COS_032" },
	{ 0xd23d28f2, "RID_COS_107" },
	{ 0xa81bd497, "MIL_COS_000" },
	{ 0xafcd79f6, "MIL_COS_001" },
	{ 0xb77f1f55, "MIL_COS_002" },
	{ 0xbf30c4b4, "MIL_COS_003" },
	{ 0xc6e26a13, "MIL_COS_004" },
	{ 0xce940f72, "MIL_COS_005" },
	{ 0xddf75a30, "MIL_COS_007" },
	{ 0x3d008758, "MIL_COS_010" },
	{ 0x44b22cb7, "MIL_COS_011" },
	{ 0x66c9ecda, "MIL_COS_030" },
	{ 0x6e7b9239, "MIL_COS_031" },
	{ 0x762d3798, "MIL_COS_032" },
	{ 0x7ddedcf7, "MIL_COS_033" },
	{ 0x85908256, "MIL_COS_034" },
	{ 0x8d4227b5, "MIL_COS_035" },
	{ 0x94f3cd14, "MIL_COS_036" },
	{ 0x9ca57273, "MIL_COS_037" },
	{ 0xac08bd31, "MIL_COS_039" },
	{ 0x9c6545b6, "MIL_COS_100" },
	{ 0xa416eb15, "MIL_COS_101" },
	{ 0xabc89074, "MIL_COS_102" },
	{ 0xb37a35d3, "MIL_COS_103" },
	{ 0xbb2bdb32, "MIL_COS_104" },
	{ 0xc2dd8091, "MIL_COS_105" },
	{ 0xca8f25f0, "MIL_COS_106" },
	{ 0xd240cb4f, "MIL_COS_107" },
	{ 0xa817a0cb, "ZAC_COS_000" },
	{ 0xafc9462a, "ZAC_COS_001" },
	{ 0xc6de3647, "ZAC_COS_004" },
	{ 0xddf32664, "ZAC_COS_007" },
	{ 0x3cfc538c, "ZAC_COS_010" },
	{ 0x44adf8eb, "ZAC_COS_011" },
	{ 0x66c5b90e, "ZAC_COS_030" },
	{ 0x762903cc, "ZAC_COS_032" },
	{ 0x9c6111ea, "ZAC_COS_100" },
	{ 0xd23c9783, "ZAC_COS_107" },
	{ 0xa81cbe2e, "TIN_COS_000" },
	{ 0xafce638d, "TIN_COS_001" },
	{ 0xb78008ec, "TIN_COS_002" },
	{ 0xbf31ae4b, "TIN_COS_003" },
	{ 0xc6e353aa, "TIN_COS_004" },
	{ 0xce94f909, "TIN_COS_005" },
	{ 0xddf843c7, "TIN_COS_007" },
	{ 0x3d0170ef, "TIN_COS_010" },
	{ 0x44b3164e, "TIN_COS_011" },
	{ 0x66cad671, "TIN_COS_030" },
	{ 0x6e7c7bd0, "TIN_COS_031" },
	{ 0x762e212f, "TIN_COS_032" },
	{ 0x7ddfc68e, "TIN_COS_033" },
	{ 0x85916bed, "TIN_COS_034" },
	{ 0x8d43114c, "TIN_COS_035" },
	{ 0x94f4b6ab, "TIN_COS_036" },
	{ 0x9ca65c0a, "TIN_COS_037" },
	{ 0xa417d4ac, "TIN_COS_101" },
	{ 0xabc97a0b, "TIN_COS_102" },
	{ 0xb37b1f6a, "TIN_COS_103" },
	{ 0xbb2cc4c9, "TIN_COS_104" },
	{ 0xc2de6a28, "TIN_COS_105" },
	{ 0xca900f87, "TIN_COS_106" },
	{ 0xd241b4e6, "TIN_COS_107" },
	{ 0xa81bc4f8, "HEL_COS_000" },
	{ 0xafcd6a57, "HEL_COS_001" },
	{ 0xb77f0fb6, "HEL_COS_002" },
	{ 0xbf30b515, "HEL_COS_003" },
	{ 0xc6e25a74, "HEL_COS_004" },
	{ 0xce93ffd3, "HEL_COS_005" },
	{ 0xddf74a91, "HEL_COS_007" },
	{ 0x3d0077b9, "HEL_COS_010" },
	{ 0x44b21d18, "HEL_COS_011" },
	{ 0x66c9dd3b, "HEL_COS_030" },
	{ 0x6e7b829a, "HEL_COS_031" },
	{ 0x762d27f9, "HEL_COS_032" },
	{ 0x7ddecd58, "HEL_COS_033" },
	{ 0x859072b7, "HEL_COS_034" },
	{ 0x8d421816, "HEL_COS_035" },
	{ 0x94f3bd75, "HEL_COS_036" },
	{ 0x9ca562d4, "HEL_COS_037" },
	{ 0xa416db76, "HEL_COS_101" },
	{ 0xabc880d5, "HEL_COS_102" },
	{ 0xb37a2634, "HEL_COS_103" },
	{ 0xbb2bcb93, "HEL_COS_104" },
	{ 0xc2dd70f2, "HEL_COS_105" },
	{ 0xca8f1651, "HEL_COS_106" },
	{ 0xd240bbb0, "HEL_COS_107" },
	{ 0xa8170f1e, "AYA_COS_000" },
	{ 0xafc8b47d, "AYA_COS_001" },
	{ 0xb77a59dc, "AYA_COS_002" },
	{ 0xbf2bff3b, "AYA_COS_003" },
	{ 0xc6dda49a, "AYA_COS_004" },
	{ 0xce8f49f9, "AYA_COS_005" },
	{ 0xddf294b7, "AYA_COS_007" },
	{ 0x3cfbc1df, "AYA_COS_010" },
	{ 0x44ad673e, "AYA_COS_011" },
	{ 0xd99219ff, "AYA_COS_021" },
	{ 0x66c52761, "AYA_COS_030" },
	{ 0x6e76ccc0, "AYA_COS_031" },
	{ 0x7628721f, "AYA_COS_032" },
	{ 0x7dda177e, "AYA_COS_033" },
	{ 0x858bbcdd, "AYA_COS_034" },
	{ 0x8d3d623c, "AYA_COS_035" },
	{ 0x94ef079b, "AYA_COS_036" },
	{ 0x9ca0acfa, "AYA_COS_037" },
	{ 0xa412259c, "AYA_COS_101" },
	{ 0xabc3cafb, "AYA_COS_102" },
	{ 0xb375705a, "AYA_COS_103" },
	{ 0xbb2715b9, "AYA_COS_104" },
	{ 0xc2d8bb18, "AYA_COS_105" },
	{ 0xca8a6077, "AYA_COS_106" },
	{ 0xd23c05d6, "AYA_COS_107" },
	{ 0xa81a81c5, "ELI_COS_000" },
	{ 0xafcc2724, "ELI_COS_001" },
	{ 0xc6e11741, "ELI_COS_004" },
	{ 0xddf6075e, "ELI_COS_007" },
	{ 0x3cff3486, "ELI_COS_010" },
	{ 0x44b0d9e5, "ELI_COS_011" },
	{ 0x66c89a08, "ELI_COS_030" },
	{ 0x762be4c6, "ELI_COS_032" },
	{ 0xd23f787d, "ELI_COS_107" },
	{ 0xa81f0311, "LIS_COS_000" },
	{ 0xafd0a870, "LIS_COS_001" },
	{ 0xb7824dcf, "LIS_COS_002" },
	{ 0xbf33f32e, "LIS_COS_003" },
	{ 0xc6e5988d, "LIS_COS_004" },
	{ 0xce973dec, "LIS_COS_005" },
	{ 0xddfa88aa, "LIS_COS_007" },
	{ 0x3d03b5d2, "LIS_COS_010" },
	{ 0x44b55b31, "LIS_COS_011" },
	{ 0x66cd1b54, "LIS_COS_030" },
	{ 0x6e7ec0b3, "LIS_COS_031" },
	{ 0x76306612, "LIS_COS_032" },
	{ 0x7de20b71, "LIS_COS_033" },
	{ 0x8593b0d0, "LIS_COS_034" },
	{ 0x8d45562f, "LIS_COS_035" },
	{ 0x94f6fb8e, "LIS_COS_036" },
	{ 0x9ca8a0ed, "LIS_COS_037" },
	{ 0xa41a198f, "LIS_COS_101" },
	{ 0xabcbbeee, "LIS_COS_102" },
	{ 0xb37d644d, "LIS_COS_103" },
	{ 0xbb2f09ac, "LIS_COS_104" },
	{ 0xc2e0af0b, "LIS_COS_105" },
	{ 0xca92546a, "LIS_COS_106" },
	{ 0xd243f9c9, "LIS_COS_107" },
	{ 0xa816f4f6, "BRA_COS_000" },
	{ 0xafc89a55, "BRA_COS_001" },
	{ 0xc6dd8a72, "BRA_COS_004" },
	{ 0xddf27a8f, "BRA_COS_007" },
	{ 0x3cfba7b7, "BRA_COS_010" },
	{ 0x44ad4d16, "BRA_COS_011" },
	{ 0x66c50d39, "BRA_COS_030" },
	{ 0x762857f7, "BRA_COS_032" },
	{ 0xd23bebae, "BRA_COS_107" },
	{ 0xa81a980d, "CRI_COS_000" },
	{ 0xafcc3d6c, "CRI_COS_001" },
	{ 0xb77de2cb, "CRI_COS_002" },
	{ 0xbf2f882a, "CRI_COS_003" },
	{ 0xc6e12d89, "CRI_COS_004" },
	{ 0xce92d2e8, "CRI_COS_005" },
	{ 0xddf61da6, "CRI_COS_007" },
	{ 0x3cff4ace, "CRI_COS_010" },
	{ 0x44b0f02d, "CRI_COS_011" },
	{ 0x66c8b050, "CRI_COS_030" },
	{ 0x6e7a55af, "CRI_COS_031" },
	{ 0x762bfb0e, "CRI_COS_032" },
	{ 0x7ddda06d, "CRI_COS_033" },
	{ 0x858f45cc, "CRI_COS_034" },
	{ 0x8d40eb2b, "CRI_COS_035" },
	{ 0x94f2908a, "CRI_COS_036" },
	{ 0x9ca435e9, "CRI_COS_037" },
	{ 0xa415ae8b, "CRI_COS_101" },
	{ 0xabc753ea, "CRI_COS_102" },
	{ 0xb378f949, "CRI_COS_103" },
	{ 0xbb2a9ea8, "CRI_COS_104" },
	{ 0xc2dc4407, "CRI_COS_105" },
	{ 0xca8de966, "CRI_COS_106" },
	{ 0xd23f8ec5, "CRI_COS_107" },
	{ 0xa8198f57, "RIG_COS_000" },
	{ 0xafcb34b6, "RIG_COS_001" },
	{ 0xc6e024d3, "RIG_COS_004" },
	{ 0xddf514f0, "RIG_COS_007" },
	{ 0x3cfe4218, "RIG_COS_010" },
	{ 0x44afe777, "RIG_COS_011" },
	{ 0x66c7a79a, "RIG_COS_030" },
	{ 0x762af258, "RIG_COS_032" },
	{ 0xd23e860f, "RIG_COS_107" },
	{ 0xa81cd340, "HON_COS_000" },
	{ 0xafce789f, "HON_COS_001" },
	{ 0xb7801dfe, "HON_COS_002" },
	{ 0xbf31c35d, "HON_COS_003" },
	{ 0xc6e368bc, "HON_COS_004" },
	{ 0xce950e1b, "HON_COS_005" },
	{ 0xd646b37a, "HON_COS_006" },
	{ 0xddf858d9, "HON_COS_007" },
	{ 0x3d018601, "HON_COS_010" },
	{ 0x44b32b60, "HON_COS_011" },
	{ 0xd1e638c2, "HON_COS_020" },
	{ 0x66caeb83, "HON_COS_030" },
	{ 0x6e7c90e2, "HON_COS_031" },
	{ 0x762e3641, "HON_COS_032" },
	{ 0x7ddfdba0, "HON_COS_033" },
	{ 0x859180ff, "HON_COS_034" },
	{ 0x8d43265e, "HON_COS_035" },
	{ 0x94f4cbbd, "HON_COS_036" },
	{ 0x9ca6711c, "HON_COS_037" },
	{ 0xa458167b, "HON_COS_038" },
	{ 0xac09bbda, "HON_COS_039" },
	{ 0xa417e9be, "HON_COS_101" },
	{ 0xabc98f1d, "HON_COS_102" },
	{ 0xb37b347c, "HON_COS_103" },
	{ 0xbb2cd9db, "HON_COS_104" },
	{ 0xc2de7f3a, "HON_COS_105" },
	{ 0xca902499, "HON_COS_106" },
	{ 0xd241c9f8, "HON_COS_107" },
	{ 0xa81d291b, "DGO_COS_000" },
	{ 0xc6e3be97, "DGO_COS_004" },
	{ 0xce9563f6, "DGO_COS_005" },
	{ 0xddf8aeb4, "DGO_COS_007" },
	{ 0x3d01dbdc, "DGO_COS_010" },
	{ 0x44b3813b, "DGO_COS_011" },
	{ 0x66cb415e, "DGO_COS_030" },
	{ 0x762e8c1c, "DGO_COS_032" },
	{ 0xd2421fd3, "DGO_COS_107" },
	{ 0xa81916f9, "PHF_COS_000" },
	{ 0xafcabc58, "PHF_COS_001" },
	{ 0xb77c61b7, "PHF_COS_002" },
	{ 0xddf49c92, "PHF_COS_007" },
	{ 0x3cfdc9ba, "PHF_COS_010" },
	{ 0x44af6f19, "PHF_COS_011" },
	{ 0x66c72f3c, "PHF_COS_030" },
	{ 0x6e78d49b, "PHF_COS_031" },
	{ 0x762a79fa, "PHF_COS_032" },
	{ 0x7ddc1f59, "PHF_COS_033" },
	{ 0x858dc4b8, "PHF_COS_034" },
	{ 0x8d3f6a17, "PHF_COS_035" },
	{ 0x94f10f76, "PHF_COS_036" },
	{ 0x9ca2b4d5, "PHF_COS_037" },
	{ 0xac05ff93, "PHF_COS_039" },
	{ 0xa4142d77, "PHF_COS_101" },
	{ 0xabc5d2d6, "PHF_COS_102" },
	{ 0xb3777835, "PHF_COS_103" },
	{ 0xbb291d94, "PHF_COS_104" },
	{ 0xc2dac2f3, "PHF_COS_105" },
	{ 0xca8c6852, "PHF_COS_106" },
	{ 0xd23e0db1, "PHF_COS_107" },
	{ 0xa81851f4, "ARD_COS_000" },
	{ 0xafc9f753, "ARD_COS_001" },
	{ 0xa81dc05a, "MPP_COS_000" },
	{ 0xafcf65b9, "MPP_COS_001" },
	{ 0xa8185422, "SRD_COS_000" },
	{ 0xafc9f981, "SRD_COS_001" },
	{ 0xa81a5972, "MAI_COS_000" },
	{ 0xc6e0eeee, "MAI_COS_004" },
	{ 0x3cff0c33, "MAI_COS_010" },
	{ 0x44b0b192, "MAI_COS_011" },
	{ 0x4c6256f1, "MAI_COS_012" },
	{ 0x5413fc50, "MAI_COS_013" },
	{ 0x5bc5a1af, "MAI_COS_014" },
	{ 0xa81b73b7, "SNK_COS_000" },
	{ 0xc6e20933, "SNK_COS_004" },
	{ 0x3d002678, "SNK_COS_010" },
	{ 0x44b1cbd7, "SNK_COS_011" },
	{ 0x4c637136, "SNK_COS_012" },
	{ 0x54151695, "SNK_COS_013" },
	{ 0x5bc6bbf4, "SNK_COS_014" },
	{ 0xa81c5f7c, "MOM_COS_000" },
	{ 0xafce04db, "MOM_COS_001" },
	{ 0xb77faa3a, "MOM_COS_002" },
	{ 0xbf314f99, "MOM_COS_003" },
	{ 0xc6e2f4f8, "MOM_COS_004" },
	{ 0xce949a57, "MOM_COS_005" },
	{ 0xddf7e515, "MOM_COS_007" },
	{ 0x3d01123d, "MOM_COS_010" },
	{ 0x44b2b79c, "MOM_COS_011" },
	{ 0x66ca77bf, "MOM_COS_030" },
	{ 0x6e7c1d1e, "MOM_COS_031" },
	{ 0x7ddf67dc, "MOM_COS_033" },
	{ 0x85910d3b, "MOM_COS_034" },
	{ 0x8d42b29a, "MOM_COS_035" },
	{ 0x94f457f9, "MOM_COS_036" },
	{ 0x9ca5fd58, "MOM_COS_037" },
	{ 0xa41775fa, "MOM_COS_101" },
	{ 0xabc91b59, "MOM_COS_102" },
	{ 0xb37ac0b8, "MOM_COS_103" },
	{ 0xbb2c6617, "MOM_COS_104" },
	{ 0xc2de0b76, "MOM_COS_105" },
	{ 0xca8fb0d5, "MOM_COS_106" },
	{ 0xd2415634, "MOM_COS_107" },
	{ 0xa8179fd3, "RAC_COS_000" },
	{ 0xafc94532, "RAC_COS_001" },
	{ 0xb77aea91, "RAC_COS_002" },
	{ 0xbf2c8ff0, "RAC_COS_003" },
	{ 0xc6de354f, "RAC_COS_004" },
	{ 0xce8fdaae, "RAC_COS_005" },
	{ 0x3cfc5294, "RAC_COS_010" },
	{ 0x44adf7f3, "RAC_COS_011" },
	{ 0x66c5b816, "RAC_COS_030" },
	{ 0x6e775d75, "RAC_COS_031" },
	{ 0x8d3df2f1, "RAC_COS_035" },
	{ 0x94ef9850, "RAC_COS_036" },
	{ 0x9ca13daf, "RAC_COS_037" },
	{ 0xb376010f, "RAC_COS_103" },
	{ 0xbb27a66e, "RAC_COS_104" },
	{ 0xc2d94bcd, "RAC_COS_105" },
	{ 0xca8af12c, "RAC_COS_106" },
	{ 0xa81839db, "SKD_COS_000" },
	{ 0xafc9df3a, "SKD_COS_001" },
	{ 0xb77b8499, "SKD_COS_002" },
	{ 0xbf2d29f8, "SKD_COS_003" },
	{ 0xc6decf57, "SKD_COS_004" },
	{ 0xce9074b6, "SKD_COS_005" },
	{ 0xddf3bf74, "SKD_COS_007" },
	{ 0x3cfcec9c, "SKD_COS_010" },
	{ 0x44ae91fb, "SKD_COS_011" },
	{ 0x66c6521e, "SKD_COS_030" },
	{ 0x6e77f77d, "SKD_COS_031" },
	{ 0xc2d9e5d5, "SKD_COS_105" },
	{ 0xca8b8b34, "SKD_COS_106" },
	{ 0x73f15cc3, "RANDOM" },
};

static const std::unordered_map<uint32_t, std::string> hash_to_hair =
{
	{ 0xc7d77b95, "KAS_HAIR_000" },
	{ 0xb65a8216, "KAS_HAIR_001" },
	{ 0xa4dd8897, "KAS_HAIR_002" },
	{ 0x93608f18, "KAS_HAIR_003" },
	{ 0x81e39599, "KAS_HAIR_004" },
	{ 0x70669c1a, "KAS_HAIR_005" },
	{ 0xcf8920f4, "KAS_HAIR_010" },
	{ 0xbe0c2775, "KAS_HAIR_011" },
	{ 0xb440d355, "KAS_HAIR_022" },
	{ 0xcd6f7233, "KAS_HAIR_031" },
	{ 0xaa757f35, "KAS_HAIR_033" },
	{ 0x877b8c37, "KAS_HAIR_035" },
	{ 0x75fe92b8, "KAS_HAIR_036" },
	{ 0x64819939, "KAS_HAIR_037" },
	{ 0x4b3f34d7, "KAS_HAIR_101" },
	{ 0x39c23b58, "KAS_HAIR_102" },
	{ 0x284541d9, "KAS_HAIR_103" },
	{ 0x054b4edb, "KAS_HAIR_105" },
	{ 0xc7d77a7e, "BAS_HAIR_000" },
	{ 0xb65a80ff, "BAS_HAIR_001" },
	{ 0xa4dd8780, "BAS_HAIR_002" },
	{ 0xbe0c265e, "BAS_HAIR_011" },
	{ 0xbbf2779d, "BAS_HAIR_032" },
	{ 0xe2515ac6, "BAS_HAIR_107" },
	{ 0x593889a4, "BAS_HAIR_107b" },
	{ 0xc7da34b8, "BAY_HAIR_000" },
	{ 0xb65d3b39, "BAY_HAIR_001" },
	{ 0xa4e041ba, "BAY_HAIR_002" },
	{ 0x3bf268c0, "BAY_HAIR_008" },
	{ 0xbe0ee098, "BAY_HAIR_011" },
	{ 0xe2541500, "BAY_HAIR_107" },
	{ 0xc7d5359b, "JAN_HAIR_000" },
	{ 0xb6583c1c, "JAN_HAIR_001" },
	{ 0xbe09e17b, "JAN_HAIR_011" },
	{ 0xc7d2ff02, "LEI_HAIR_000" },
	{ 0xb6560583, "LEI_HAIR_001" },
	{ 0xa4d90c04, "LEI_HAIR_002" },
	{ 0x935c1285, "LEI_HAIR_003" },
	{ 0x81df1906, "LEI_HAIR_004" },
	{ 0x70621f87, "LEI_HAIR_005" },
	{ 0xcf84a461, "LEI_HAIR_010" },
	{ 0xbe07aae2, "LEI_HAIR_011" },
	{ 0xcd6af5a0, "LEI_HAIR_031" },
	{ 0xaa7102a2, "LEI_HAIR_033" },
	{ 0x98f40923, "LEI_HAIR_034" },
	{ 0x87770fa4, "LEI_HAIR_035" },
	{ 0x75fa1625, "LEI_HAIR_036" },
	{ 0x647d1ca6, "LEI_HAIR_037" },
	{ 0x39bdbec5, "LEI_HAIR_102" },
	{ 0x2840c546, "LEI_HAIR_103" },
	{ 0x16c3cbc7, "LEI_HAIR_104" },
	{ 0xe24cdf4a, "LEI_HAIR_107" },
	{ 0xc7d50851, "HTM_HAIR_000" },
	{ 0xb6580ed2, "HTM_HAIR_001" },
	{ 0xa4db1553, "HTM_HAIR_002" },
	{ 0x935e1bd4, "HTM_HAIR_003" },
	{ 0x81e12255, "HTM_HAIR_004" },
	{ 0x706428d6, "HTM_HAIR_005" },
	{ 0x5ee72f57, "HTM_HAIR_006" },
	{ 0x4d6a35d8, "HTM_HAIR_007" },
	{ 0x3bed3c59, "HTM_HAIR_008" },
	{ 0x2a7042da, "HTM_HAIR_009" },
	{ 0xcf86adb0, "HTM_HAIR_010" },
	{ 0xbe09b431, "HTM_HAIR_011" },
	{ 0xcd6cfeef, "HTM_HAIR_031" },
	{ 0xaa730bf1, "HTM_HAIR_033" },
	{ 0x98f61272, "HTM_HAIR_034" },
	{ 0x877918f3, "HTM_HAIR_035" },
	{ 0x75fc1f74, "HTM_HAIR_036" },
	{ 0x647f25f5, "HTM_HAIR_037" },
	{ 0x4b3cc193, "HTM_HAIR_101" },
	{ 0x39bfc814, "HTM_HAIR_102" },
	{ 0x2842ce95, "HTM_HAIR_103" },
	{ 0x16c5d516, "HTM_HAIR_104" },
	{ 0xe24ee899, "HTM_HAIR_107" },
	{ 0xc7d8bf44, "RYU_HAIR_000" },
	{ 0xb65bc5c5, "RYU_HAIR_001" },
	{ 0xbe0d6b24, "RYU_HAIR_011" },
	{ 0x5cbd7205, "RYU_HAIR_100" },
	{ 0xe2529f8c, "RYU_HAIR_107" },
	{ 0xc7d849af, "HYT_HAIR_000" },
	{ 0xb65b5030, "HYT_HAIR_001" },
	{ 0xa4de56b1, "HYT_HAIR_002" },
	{ 0xbe0cf58f, "HYT_HAIR_011" },
	{ 0xbbf346ce, "HYT_HAIR_032" },
	{ 0x5cbcfc70, "HYT_HAIR_100" },
	{ 0xc7d70774, "MAR_HAIR_000" },
	{ 0xb65a0df5, "MAR_HAIR_001" },
	{ 0xa4dd1476, "MAR_HAIR_002" },
	{ 0x93601af7, "MAR_HAIR_003" },
	{ 0x81e32178, "MAR_HAIR_004" },
	{ 0x706627f9, "MAR_HAIR_005" },
	{ 0x5ee92e7a, "MAR_HAIR_006" },
	{ 0x4d6c34fb, "MAR_HAIR_007" },
	{ 0xcf88acd3, "MAR_HAIR_010" },
	{ 0xbe0bb354, "MAR_HAIR_011" },
	{ 0xb4405f34, "MAR_HAIR_022" },
	{ 0xcd6efe12, "MAR_HAIR_031" },
	{ 0xaa750b14, "MAR_HAIR_033" },
	{ 0x877b1816, "MAR_HAIR_035" },
	{ 0x75fe1e97, "MAR_HAIR_036" },
	{ 0x64812518, "MAR_HAIR_037" },
	{ 0x53042b99, "MAR_HAIR_038" },
	{ 0x4187321a, "MAR_HAIR_039" },
	{ 0x5cbbba35, "MAR_HAIR_100" },
	{ 0x4b3ec0b6, "MAR_HAIR_101" },
	{ 0x39c1c737, "MAR_HAIR_102" },
	{ 0x2844cdb8, "MAR_HAIR_103" },
	{ 0x16c7d439, "MAR_HAIR_104" },
	{ 0xc7d0540a, "NIC_HAIR_000" },
	{ 0xb6535a8b, "NIC_HAIR_001" },
	{ 0xa4d6610c, "NIC_HAIR_002" },
	{ 0x9359678d, "NIC_HAIR_003" },
	{ 0x81dc6e0e, "NIC_HAIR_004" },
	{ 0x705f748f, "NIC_HAIR_005" },
	{ 0x5ee27b10, "NIC_HAIR_006" },
	{ 0x4d658191, "NIC_HAIR_007" },
	{ 0xcf81f969, "NIC_HAIR_010" },
	{ 0xbe04ffea, "NIC_HAIR_011" },
	{ 0xcd684aa8, "NIC_HAIR_031" },
	{ 0xaa6e57aa, "NIC_HAIR_033" },
	{ 0x98f15e2b, "NIC_HAIR_034" },
	{ 0x877464ac, "NIC_HAIR_035" },
	{ 0x75f76b2d, "NIC_HAIR_036" },
	{ 0x647a71ae, "NIC_HAIR_037" },
	{ 0x52fd782f, "NIC_HAIR_038" },
	{ 0x41807eb0, "NIC_HAIR_039" },
	{ 0x4b380d4c, "NIC_HAIR_101" },
	{ 0x39bb13cd, "NIC_HAIR_102" },
	{ 0x283e1a4e, "NIC_HAIR_103" },
	{ 0x16c120cf, "NIC_HAIR_104" },
	{ 0x05442750, "NIC_HAIR_105" },
	{ 0xe24a3452, "NIC_HAIR_107" },
	{ 0xc7d40d2b, "KOK_HAIR_000" },
	{ 0xb65713ac, "KOK_HAIR_001" },
	{ 0xa4da1a2d, "KOK_HAIR_002" },
	{ 0x935d20ae, "KOK_HAIR_003" },
	{ 0x81e0272f, "KOK_HAIR_004" },
	{ 0x70632db0, "KOK_HAIR_005" },
	{ 0xcf85b28a, "KOK_HAIR_010" },
	{ 0xbe08b90b, "KOK_HAIR_011" },
	{ 0xcd6c03c9, "KOK_HAIR_031" },
	{ 0xaa7210cb, "KOK_HAIR_033" },
	{ 0x87781dcd, "KOK_HAIR_035" },
	{ 0x75fb244e, "KOK_HAIR_036" },
	{ 0x647e2acf, "KOK_HAIR_037" },
	{ 0x4b3bc66d, "KOK_HAIR_101" },
	{ 0x39beccee, "KOK_HAIR_102" },
	{ 0x2841d36f, "KOK_HAIR_103" },
	{ 0x16c4d9f0, "KOK_HAIR_104" },
	{ 0x0547e071, "KOK_HAIR_105" },
	{ 0xe24ded73, "KOK_HAIR_107" },
	{ 0xc7d6048e, "NYO_HAIR_000" },
	{ 0xb6590b0f, "NYO_HAIR_001" },
	{ 0xa4dc1190, "NYO_HAIR_002" },
	{ 0x935f1811, "NYO_HAIR_003" },
	{ 0x81e21e92, "NYO_HAIR_004" },
	{ 0x70652513, "NYO_HAIR_005" },
	{ 0x5ee82b94, "NYO_HAIR_006" },
	{ 0x4d6b3215, "NYO_HAIR_007" },
	{ 0x3bee3896, "NYO_HAIR_008" },
	{ 0xcf87a9ed, "NYO_HAIR_010" },
	{ 0xbe0ab06e, "NYO_HAIR_011" },
	{ 0xcd6dfb2c, "NYO_HAIR_031" },
	{ 0xaa74082e, "NYO_HAIR_033" },
	{ 0x98f70eaf, "NYO_HAIR_034" },
	{ 0x877a1530, "NYO_HAIR_035" },
	{ 0x75fd1bb1, "NYO_HAIR_036" },
	{ 0x64802232, "NYO_HAIR_037" },
	{ 0x39c0c451, "NYO_HAIR_102" },
	{ 0x2843cad2, "NYO_HAIR_103" },
	{ 0xc7d0c8e5, "RID_HAIR_000" },
	{ 0xbe0574c5, "RID_HAIR_011" },
	{ 0xe24aa92d, "RID_HAIR_107" },
	{ 0xc7d46b42, "MIL_HAIR_000" },
	{ 0xb65771c3, "MIL_HAIR_001" },
	{ 0xa4da7844, "MIL_HAIR_002" },
	{ 0x935d7ec5, "MIL_HAIR_003" },
	{ 0x81e08546, "MIL_HAIR_004" },
	{ 0x70638bc7, "MIL_HAIR_005" },
	{ 0x5ee69248, "MIL_HAIR_006" },
	{ 0x3bec9f4a, "MIL_HAIR_008" },
	{ 0xcf8610a1, "MIL_HAIR_010" },
	{ 0xbe091722, "MIL_HAIR_011" },
	{ 0xcd6c61e0, "MIL_HAIR_031" },
	{ 0xaa726ee2, "MIL_HAIR_033" },
	{ 0x87787be4, "MIL_HAIR_035" },
	{ 0x75fb8265, "MIL_HAIR_036" },
	{ 0x647e88e6, "MIL_HAIR_037" },
	{ 0x4b3c2484, "MIL_HAIR_101" },
	{ 0x39bf2b05, "MIL_HAIR_102" },
	{ 0x28423186, "MIL_HAIR_103" },
	{ 0x16c53807, "MIL_HAIR_104" },
	{ 0xe24e4b8a, "MIL_HAIR_107" },
	{ 0xc7d03776, "ZAC_HAIR_000" },
	{ 0xbe04e356, "ZAC_HAIR_011" },
	{ 0xbbeb3495, "ZAC_HAIR_032" },
	{ 0xe24a17be, "ZAC_HAIR_107" },
	{ 0xc7d554d9, "TIN_HAIR_000" },
	{ 0xb6585b5a, "TIN_HAIR_001" },
	{ 0xa4db61db, "TIN_HAIR_002" },
	{ 0x935e685c, "TIN_HAIR_003" },
	{ 0x81e16edd, "TIN_HAIR_004" },
	{ 0x3bed88e1, "TIN_HAIR_008" },
	{ 0xcf86fa38, "TIN_HAIR_010" },
	{ 0xbe0a00b9, "TIN_HAIR_011" },
	{ 0xcd6d4b77, "TIN_HAIR_031" },
	{ 0xaa735879, "TIN_HAIR_033" },
	{ 0x98f65efa, "TIN_HAIR_034" },
	{ 0x8779657b, "TIN_HAIR_035" },
	{ 0x647f727d, "TIN_HAIR_037" },
	{ 0x4b3d0e1b, "TIN_HAIR_101" },
	{ 0x39c0149c, "TIN_HAIR_102" },
	{ 0x28431b1d, "TIN_HAIR_103" },
	{ 0x16c6219e, "TIN_HAIR_104" },
	{ 0xe24f3521, "TIN_HAIR_107" },
	{ 0xc7d45ba3, "HEL_HAIR_000" },
	{ 0xb6576224, "HEL_HAIR_001" },
	{ 0xa4da68a5, "HEL_HAIR_002" },
	{ 0x935d6f26, "HEL_HAIR_003" },
	{ 0x81e075a7, "HEL_HAIR_004" },
	{ 0x70637c28, "HEL_HAIR_005" },
	{ 0xcf860102, "HEL_HAIR_010" },
	{ 0xbe090783, "HEL_HAIR_011" },
	{ 0xcd6c5241, "HEL_HAIR_031" },
	{ 0xaa725f43, "HEL_HAIR_033" },
	{ 0x87786c45, "HEL_HAIR_035" },
	{ 0x647e7947, "HEL_HAIR_037" },
	{ 0x4b3c14e5, "HEL_HAIR_101" },
	{ 0x39bf1b66, "HEL_HAIR_102" },
	{ 0x284221e7, "HEL_HAIR_103" },
	{ 0xe24e3beb, "HEL_HAIR_107" },
	{ 0xc7cfa5c9, "AYA_HAIR_000" },
	{ 0xb652ac4a, "AYA_HAIR_001" },
	{ 0xa4d5b2cb, "AYA_HAIR_002" },
	{ 0x9358b94c, "AYA_HAIR_003" },
	{ 0x81dbbfcd, "AYA_HAIR_004" },
	{ 0x705ec64e, "AYA_HAIR_005" },
	{ 0xcf814b28, "AYA_HAIR_010" },
	{ 0xbe0451a9, "AYA_HAIR_011" },
	{ 0xcd679c67, "AYA_HAIR_031" },
	{ 0xaa6da969, "AYA_HAIR_033" },
	{ 0x8773b66b, "AYA_HAIR_035" },
	{ 0x75f6bcec, "AYA_HAIR_036" },
	{ 0x6479c36d, "AYA_HAIR_037" },
	{ 0x39ba658c, "AYA_HAIR_102" },
	{ 0x283d6c0d, "AYA_HAIR_103" },
	{ 0x16c0728e, "AYA_HAIR_104" },
	{ 0xc7d31870, "ELI_HAIR_000" },
	{ 0xb6561ef1, "ELI_HAIR_001" },
	{ 0xa4d92572, "ELI_HAIR_002" },
	{ 0x935c2bf3, "ELI_HAIR_003" },
	{ 0xbe07c450, "ELI_HAIR_011" },
	{ 0xe24cf8b8, "ELI_HAIR_107" },
	{ 0xc7d799bc, "LIS_HAIR_000" },
	{ 0xb65aa03d, "LIS_HAIR_001" },
	{ 0xa4dda6be, "LIS_HAIR_002" },
	{ 0x9360ad3f, "LIS_HAIR_003" },
	{ 0x81e3b3c0, "LIS_HAIR_004" },
	{ 0x7066ba41, "LIS_HAIR_005" },
	{ 0x3befcdc4, "LIS_HAIR_008" },
	{ 0xcf893f1b, "LIS_HAIR_010" },
	{ 0xbe0c459c, "LIS_HAIR_011" },
	{ 0xcd6f905a, "LIS_HAIR_031" },
	{ 0xaa759d5c, "LIS_HAIR_033" },
	{ 0x877baa5e, "LIS_HAIR_035" },
	{ 0x6481b760, "LIS_HAIR_037" },
	{ 0x4b3f52fe, "LIS_HAIR_101" },
	{ 0x39c2597f, "LIS_HAIR_102" },
	{ 0x28456000, "LIS_HAIR_103" },
	{ 0x16c86681, "LIS_HAIR_104" },
	{ 0x054b6d02, "LIS_HAIR_105" },
	{ 0xe2517a04, "LIS_HAIR_107" },
	{ 0xc7cf8ba1, "BRA_HAIR_000" },
	{ 0xb6529222, "BRA_HAIR_001" },
	{ 0xa4d598a3, "BRA_HAIR_002" },
	{ 0xbe043781, "BRA_HAIR_011" },
	{ 0xc7d32eb8, "CRI_HAIR_000" },
	{ 0xb6563539, "CRI_HAIR_001" },
	{ 0xa4d93bba, "CRI_HAIR_002" },
	{ 0x935c423b, "CRI_HAIR_003" },
	{ 0xcf84d417, "CRI_HAIR_010" },
	{ 0xbe07da98, "CRI_HAIR_011" },
	{ 0xcd6b2556, "CRI_HAIR_031" },
	{ 0xaa713258, "CRI_HAIR_033" },
	{ 0x98f438d9, "CRI_HAIR_034" },
	{ 0x87773f5a, "CRI_HAIR_035" },
	{ 0x647d4c5c, "CRI_HAIR_037" },
	{ 0x4b3ae7fa, "CRI_HAIR_101" },
	{ 0x39bdee7b, "CRI_HAIR_102" },
	{ 0x2840f4fc, "CRI_HAIR_103" },
	{ 0x16c3fb7d, "CRI_HAIR_104" },
	{ 0xe24d0f00, "CRI_HAIR_107" },
	{ 0xc7d22602, "RIG_HAIR_000" },
	{ 0xbe06d1e2, "RIG_HAIR_011" },
	{ 0xbbed2321, "RIG_HAIR_032" },
	{ 0xe24c064a, "RIG_HAIR_107" },
	{ 0xc7d569eb, "HON_HAIR_000" },
	{ 0xb658706c, "HON_HAIR_001" },
	{ 0xa4db76ed, "HON_HAIR_002" },
	{ 0x935e7d6e, "HON_HAIR_003" },
	{ 0x81e183ef, "HON_HAIR_004" },
	{ 0xcf870f4a, "HON_HAIR_010" },
	{ 0xbe0a15cb, "HON_HAIR_011" },
	{ 0xcd6d6089, "HON_HAIR_031" },
	{ 0xaa736d8b, "HON_HAIR_033" },
	{ 0x98f6740c, "HON_HAIR_034" },
	{ 0x87797a8d, "HON_HAIR_035" },
	{ 0x75fc810e, "HON_HAIR_036" },
	{ 0x647f878f, "HON_HAIR_037" },
	{ 0x53028e10, "HON_HAIR_038" },
	{ 0x41859491, "HON_HAIR_039" },
	{ 0x5cba1cac, "HON_HAIR_100" },
	{ 0x39c029ae, "HON_HAIR_102" },
	{ 0x2843302f, "HON_HAIR_103" },
	{ 0x16c636b0, "HON_HAIR_104" },
	{ 0xe24f4a33, "HON_HAIR_107" },
	{ 0xc7d5bfc6, "DGO_HAIR_000" },
	{ 0xb658c647, "DGO_HAIR_001" },
	{ 0xbe0a6ba6, "DGO_HAIR_011" },
	{ 0xbbf0bce5, "DGO_HAIR_032" },
	{ 0xe24fa00e, "DGO_HAIR_107" },
	{ 0xc7d1ada4, "PHF_HAIR_000" },
	{ 0xb654b425, "PHF_HAIR_001" },
	{ 0xa4d7baa6, "PHF_HAIR_002" },
	{ 0x935ac127, "PHF_HAIR_003" },
	{ 0x81ddc7a8, "PHF_HAIR_004" },
	{ 0x7060ce29, "PHF_HAIR_005" },
	{ 0xcf835303, "PHF_HAIR_010" },
	{ 0xbe065984, "PHF_HAIR_011" },
	{ 0xcd69a442, "PHF_HAIR_031" },
	{ 0xaa6fb144, "PHF_HAIR_033" },
	{ 0x8775be46, "PHF_HAIR_035" },
	{ 0x75f8c4c7, "PHF_HAIR_036" },
	{ 0x647bcb48, "PHF_HAIR_037" },
	{ 0x4181d84a, "PHF_HAIR_039" },
	{ 0x4b3966e6, "PHF_HAIR_101" },
	{ 0x39bc6d67, "PHF_HAIR_102" },
	{ 0x283f73e8, "PHF_HAIR_103" },
	{ 0x16c27a69, "PHF_HAIR_104" },
	{ 0x054580ea, "PHF_HAIR_105" },
	{ 0xe24b8dec, "PHF_HAIR_107" },
	{ 0xc7d0e89f, "ARD_HAIR_000" },
	{ 0xb6595d86, "MPP_HAIR_001" },
	{ 0xa4dc6407, "MPP_HAIR_002" },
	{ 0xc7d0eacd, "SRD_HAIR_000" },
	{ 0xc7d2f01d, "MAI_HAIR_000" },
	{ 0xb655f69e, "MAI_HAIR_001" },
	{ 0xcf84957c, "MAI_HAIR_010" },
	{ 0xbe079bfd, "MAI_HAIR_011" },
	{ 0xac8aa27e, "MAI_HAIR_012" },
	{ 0x9b0da8ff, "MAI_HAIR_013" },
	{ 0x8990af80, "MAI_HAIR_014" },
	{ 0xc7d40a62, "SNK_HAIR_000" },
	{ 0xb65710e3, "SNK_HAIR_001" },
	{ 0xcf85afc1, "SNK_HAIR_010" },
	{ 0xbe08b642, "SNK_HAIR_011" },
	{ 0xac8bbcc3, "SNK_HAIR_012" },
	{ 0x9b0ec344, "SNK_HAIR_013" },
	{ 0x8991c9c5, "SNK_HAIR_014" },
	{ 0xc7d4f627, "MOM_HAIR_000" },
	{ 0xb657fca8, "MOM_HAIR_001" },
	{ 0xa4db0329, "MOM_HAIR_002" },
	{ 0x935e09aa, "MOM_HAIR_003" },
	{ 0x81e1102b, "MOM_HAIR_004" },
	{ 0x706416ac, "MOM_HAIR_005" },
	{ 0x5ee71d2d, "MOM_HAIR_006" },
	{ 0xcf869b86, "MOM_HAIR_010" },
	{ 0xbe09a207, "MOM_HAIR_011" },
	{ 0xcd6cecc5, "MOM_HAIR_031" },
	{ 0xaa72f9c7, "MOM_HAIR_033" },
	{ 0x877906c9, "MOM_HAIR_035" },
	{ 0x647f13cb, "MOM_HAIR_037" },
	{ 0x4b3caf69, "MOM_HAIR_101" },
	{ 0x39bfb5ea, "MOM_HAIR_102" },
	{ 0x2842bc6b, "MOM_HAIR_103" },
	{ 0xc7d0367e, "RAC_HAIR_000" },
	{ 0xb6533cff, "RAC_HAIR_001" },
	{ 0xa4d64380, "RAC_HAIR_002" },
	{ 0x93594a01, "RAC_HAIR_003" },
	{ 0x81dc5082, "RAC_HAIR_004" },
	{ 0x705f5703, "RAC_HAIR_005" },
	{ 0x5ee25d84, "RAC_HAIR_006" },
	{ 0xcf81dbdd, "RAC_HAIR_010" },
	{ 0xcd682d1c, "RAC_HAIR_031" },
	{ 0x87744720, "RAC_HAIR_035" },
	{ 0x647a5422, "RAC_HAIR_037" },
	{ 0x283dfcc2, "RAC_HAIR_103" },
	{ 0xc7d0d086, "SKD_HAIR_000" },
	{ 0xb653d707, "SKD_HAIR_001" },
	{ 0xbe057c66, "SKD_HAIR_011" },
	{ 0xcd68c724, "SKD_HAIR_031" },
};

static const std::unordered_map<uint32_t, std::string> hash_to_face =
{
	{ 0xcdad1a10, "KAS_FACE_000" },
	{ 0xbc302091, "KAS_FACE_001" },
	{ 0x41c54e18, "KAS_FACE_008" },
	{ 0xc1c8172f, "KAS_FACE_032" },
	{ 0xcdad18f9, "BAS_FACE_000" },
	{ 0xbc301f7a, "BAS_FACE_001" },
	{ 0xc3e1c4d9, "BAS_FACE_011" },
	{ 0xcdafd333, "BAY_FACE_000" },
	{ 0xbc32d9b4, "BAY_FACE_001" },
	{ 0xc1cad052, "BAY_FACE_032" },
	{ 0xcdaad416, "JAN_FACE_000" },
	{ 0xbc2dda97, "JAN_FACE_001" },
	{ 0xc1c5d135, "JAN_FACE_032" },
	{ 0xcda89d7d, "LEI_FACE_000" },
	{ 0xbc2ba3fe, "LEI_FACE_001" },
	{ 0xc1c39a9c, "LEI_FACE_032" },
	{ 0xcdaaa6cc, "HTM_FACE_000" },
	{ 0xbc2dad4d, "HTM_FACE_001" },
	{ 0xc1c5a3eb, "HTM_FACE_032" },
	{ 0xcdae5dbf, "RYU_FACE_000" },
	{ 0xbc316440, "RYU_FACE_001" },
	{ 0xaab46ac1, "RYU_FACE_002" },
	{ 0x99377142, "RYU_FACE_003" },
	{ 0xc1c95ade, "RYU_FACE_032" },
	{ 0xe8283e07, "RYU_FACE_107" },
	{ 0xcdade82a, "HYT_FACE_000" },
	{ 0xbc30eeab, "HYT_FACE_001" },
	{ 0xc1c8e549, "HYT_FACE_032" },
	{ 0x62929aeb, "HYT_FACE_100" },
	{ 0xcdaca5ef, "MAR_FACE_000" },
	{ 0xbc2fac70, "MAR_FACE_001" },
	{ 0xc3e151cf, "MAR_FACE_011" },
	{ 0xc1c7a30e, "MAR_FACE_032" },
	{ 0xcda5f285, "NIC_FACE_000" },
	{ 0xbc28f906, "NIC_FACE_001" },
	{ 0xc3da9e65, "NIC_FACE_011" },
	{ 0xc1c0efa4, "NIC_FACE_032" },
	{ 0xcda9aba6, "KOK_FACE_000" },
	{ 0xbc2cb227, "KOK_FACE_001" },
	{ 0xc1c4a8c5, "KOK_FACE_032" },
	{ 0xcdaba309, "NYO_FACE_000" },
	{ 0xbc2ea98a, "NYO_FACE_001" },
	{ 0xc1c6a028, "NYO_FACE_032" },
	{ 0xcda66760, "RID_FACE_000" },
	{ 0xbc296de1, "RID_FACE_001" },
	{ 0x41be9b68, "RID_FACE_008" },
	{ 0xc1c1647f, "RID_FACE_032" },
	{ 0xcdaa09bd, "MIL_FACE_000" },
	{ 0xbc2d103e, "MIL_FACE_001" },
	{ 0xc1c506dc, "MIL_FACE_032" },
	{ 0xcda5d5f1, "ZAC_FACE_000" },
	{ 0xbc28dc72, "ZAC_FACE_001" },
	{ 0xc3da81d1, "ZAC_FACE_011" },
	{ 0xcdaaf354, "TIN_FACE_000" },
	{ 0xbc2df9d5, "TIN_FACE_001" },
	{ 0xc1c5f073, "TIN_FACE_032" },
	{ 0xcda9fa1e, "HEL_FACE_000" },
	{ 0xbc2d009f, "HEL_FACE_001" },
	{ 0xc1c4f73d, "HEL_FACE_032" },
	{ 0xcda54444, "AYA_FACE_000" },
	{ 0xbc284ac5, "AYA_FACE_001" },
	{ 0xc1c04163, "AYA_FACE_032" },
	{ 0xcda8b6eb, "ELI_FACE_000" },
	{ 0xbc2bbd6c, "ELI_FACE_001" },
	{ 0xcdad3837, "LIS_FACE_000" },
	{ 0xbc303eb8, "LIS_FACE_001" },
	{ 0xaab34539, "LIS_FACE_002" },
	{ 0x41c56c3f, "LIS_FACE_008" },
	{ 0xc1c83556, "LIS_FACE_032" },
	{ 0xcda52a1c, "BRA_FACE_000" },
	{ 0xbc28309d, "BRA_FACE_001" },
	{ 0xc3d9d5fc, "BRA_FACE_011" },
	{ 0xc1c0273b, "BRA_FACE_032" },
	{ 0xcda8cd33, "CRI_FACE_000" },
	{ 0xbc2bd3b4, "CRI_FACE_001" },
	{ 0xc1c3ca52, "CRI_FACE_032" },
	{ 0xcda7c47d, "RIG_FACE_000" },
	{ 0xbc2acafe, "RIG_FACE_001" },
	{ 0xc1c2c19c, "RIG_FACE_032" },
	{ 0xcdab0866, "HON_FACE_000" },
	{ 0xbc2e0ee7, "HON_FACE_001" },
	{ 0xc1c60585, "HON_FACE_032" },
	{ 0xcdab5e41, "DGO_FACE_000" },
	{ 0xbc2e64c2, "DGO_FACE_001" },
	{ 0xc3e00a21, "DGO_FACE_011" },
	{ 0xc1c65b60, "DGO_FACE_032" },
	{ 0xcda74c1f, "PHF_FACE_000" },
	{ 0xbc2a52a0, "PHF_FACE_001" },
	{ 0x475776c5, "PHF_FACE_039" },
	{ 0xcda6871a, "ARD_FACE_000" },
	{ 0xbc298d9b, "ARD_FACE_001" },
	{ 0xcdabf580, "MPP_FACE_000" },
	{ 0xbc2efc01, "MPP_FACE_001" },
	{ 0xcda68948, "SRD_FACE_000" },
	{ 0xbc298fc9, "SRD_FACE_001" },
	{ 0xcda88e98, "MAI_FACE_000" },
	{ 0xbc2b9519, "MAI_FACE_001" },
	{ 0xcda9a8dd, "SNK_FACE_000" },
	{ 0xbc2caf5e, "SNK_FACE_001" },
	{ 0xcdaa94a2, "MOM_FACE_000" },
	{ 0xbc2d9b23, "MOM_FACE_001" },
	{ 0xcda5d4f9, "RAC_FACE_000" },
	{ 0xbc28db7a, "RAC_FACE_001" },
	{ 0xc3da80d9, "RAC_FACE_011" },
	{ 0xcda66f01, "SKD_FACE_000" },
	{ 0xbc297582, "SKD_FACE_001" },
};

static const std::unordered_map<uint32_t, std::string> hash_to_glasses =
{
	{ 0xcb340da3, "ITM_GLASSES_KAS_001" },
	{ 0xb2d5e402, "ITM_GLASSES_KAS_002" },
	{ 0x9a77ba61, "ITM_GLASSES_KAS_003" },
	{ 0xda67f70c, "ITM_GLASSES_BAS_001" },
	{ 0xc209cd6b, "ITM_GLASSES_BAS_002" },
	{ 0x23d32a46, "ITM_GLASSES_BAY_001" },
	{ 0x0b7500a5, "ITM_GLASSES_BAY_002" },
	{ 0x817ceea9, "ITM_GLASSES_JAN_001" },
	{ 0x691ec508, "ITM_GLASSES_JAN_002" },
	{ 0x44ccd590, "ITM_GLASSES_LEI_001" },
	{ 0x8778605f, "ITM_GLASSES_HTM_001" },
	{ 0x27fa42d2, "ITM_GLASSES_RYU_001" },
	{ 0x73be84bd, "ITM_GLASSES_HYT_001" },
	{ 0xad5dc302, "ITM_GLASSES_MAR_001" },
	{ 0x94ff9961, "ITM_GLASSES_MAR_002" },
	{ 0x7ca16fc0, "ITM_GLASSES_MAR_003" },
	{ 0x268ade98, "ITM_GLASSES_NIC_001" },
	{ 0x0e2cb4f7, "ITM_GLASSES_NIC_002" },
	{ 0xf5ce8b56, "ITM_GLASSES_NIC_003" },
	{ 0xa8a4ee39, "ITM_GLASSES_KOK_001" },
	{ 0x5717e91c, "ITM_GLASSES_NYO_001" },
	{ 0xa6f5c746, "ITM_GLASSES_NYO_002_MASK" },
	{ 0x8e979da5, "ITM_GLASSES_NYO_003_MASK" },
	{ 0x8f93e2f3, "ITM_GLASSES_RID_001" },
	{ 0x32cde1d0, "ITM_GLASSES_MIL_001" },
	{ 0x1a6fb82f, "ITM_GLASSES_MIL_002" },
	{ 0xee150004, "ITM_GLASSES_ZAC_001" },
	{ 0xd5b6d663, "ITM_GLASSES_ZAC_002" },
	{ 0xbd58acc2, "ITM_GLASSES_ZAC_003" },
	{ 0xa4fa8321, "ITM_GLASSES_ZAC_004" },
	{ 0x230220e7, "ITM_GLASSES_TIN_001" },
	{ 0x0aa3f746, "ITM_GLASSES_TIN_002" },
	{ 0xf245cda5, "ITM_GLASSES_TIN_003" },
	{ 0xd9e7a404, "ITM_GLASSES_TIN_004" },
	{ 0x620b48b1, "ITM_GLASSES_HEL_001" },
	{ 0x49ad1f10, "ITM_GLASSES_HEL_002" },
	{ 0x314ef56f, "ITM_GLASSES_HEL_003" },
	{ 0x88da89d7, "ITM_GLASSES_AYA_001" },
	{ 0x8cbc39fe, "ITM_GLASSES_ELI_001" },
	{ 0x7bed294a, "ITM_GLASSES_LIS_001" },
	{ 0x638effa9, "ITM_GLASSES_LIS_002" },
	{ 0x4b30d608, "ITM_GLASSES_LIS_003" },
	{ 0x32d2ac67, "ITM_GLASSES_LIS_004" },
	{ 0x1a7482c6, "ITM_GLASSES_LIS_005" },
	{ 0x02165925, "ITM_GLASSES_LIS_006" },
	{ 0xf5b86baf, "ITM_GLASSES_BRA_001" },
	{ 0xe4252446, "ITM_GLASSES_CRI_001" },
	{ 0xcbc6faa5, "ITM_GLASSES_CRI_002" },
	{ 0x34497c90, "ITM_GLASSES_RIG_001" },
	{ 0xa7c12af9, "ITM_GLASSES_HON_001" },
	{ 0x8f630158, "ITM_GLASSES_HON_002" },
	{ 0x33009054, "ITM_GLASSES_DGO_001" },
	{ 0x1aa266b3, "ITM_GLASSES_DGO_002" },
	{ 0x3520a132, "ITM_GLASSES_PHF_001" },
	{ 0x2f843d35, "ITM_GLASSES_MOM_001" },
	{ 0xdf26b30c, "ITM_GLASSES_RAC_001" },
	{ 0xc6c8896b, "ITM_GLASSES_RAC_002" },
	{ 0x25288114, "ITM_GLASSES_SKD_001" },
};

static const std::unordered_map<uint32_t, std::string> hash_to_stage =
{
	{ 0xfb6cc144, "S0101PIR" }, // Forbidden fortune
	{ 0xfd2199e3, "S0102PIR" },
	{ 0x099259c5, "S0199PIR" },
	{ 0x4e2a96cd, "S0201BST" }, // The throwdown
	{ 0x0d511087, "S0301CRM" }, // Chinese festival
	{ 0x0f05e926, "S0302CRM" },
	{ 0x1b76a908, "S0399CRM" },
	{ 0x17a511b2, "S0401CLS" }, // Colosseum
	{ 0x17b32933, "S0411CLS" },
	{ 0x50a99ac9, "S0501PAS" }, // A.P.O
	{ 0x525e7368, "S0502PAS" },
	{ 0x5ecf334a, "S0599PAS" },
	{ 0xff9c23fe, "S0601LBN" }, // Zero
	{ 0xcf8f5330, "S0701MUS" }, // Unforgettable
	{ 0xd1442bcf, "S0702MUS" },
	{ 0xddb4ebb1, "S0799MUS" },
	{ 0x2b627f14, "S0801LOS" }, // Lost paradise
	{ 0x2d1757b3, "S0802LOS" },
	{ 0x39881795, "S0899LOS" },
	{ 0x3c91a512, "S0901WAY" }, // Road Rage
	{ 0xf275f57a, "S1101BAM" }, // Miyabi
	{ 0xf42ace19, "S1102BAM" }, // Hidden forest
	{ 0x009b8dfb, "S1199BAM" },
	{ 0xe58a8038, "S1201PRO" }, // The muscle
	{ 0xb8123b65, "S1301GYM" }, // Sweat
	{ 0xaa830301, "S1501GRD" }, // Chamber of potential
	{ 0x1e5492c9, "S1601ISL" }, // Zack island (Seaside Eden)
	{ 0x2c7a2b4a, "S1699ISL" }, 
	{ 0x04b88a2d, "BOSS" }, // Pseudo boss stage (it is really S1102BAM, but will activate night mode)
	{ 0x8668d961, "BOSSL" }, // Same as above but with more light.
	{ 0x0aa42598, "COLNIGHT" }, // Pseudo colossem night stage (it is really S0401CLS, but will activate night mode)
	{ 0x73f4de08, "RRNIGHT" }, // Pseudo Road Rage night stage (it is really S0901WAY, but will activate night mode)
	{ 0x076a493b, "THROWA" }, // Pseudo Throwdown afternoon (it is really S0201BST, but will activate afternoon mode)
};

static const std::unordered_map<uint32_t, uint32_t> final_stages =
{
	{ 0xfb6cc144, 0xfd2199e3 }, // S0101PIR -> S0102PIR
	{ 0xfd2199e3, 0x099259c5 }, // S0102PIR -> S0199PIR
	{ 0x2b627f14, 0x2d1757b3 }, // S0801LOS -> S0802LOS
	{ 0x2d1757b3, 0x2b627f14 }, // S0802LOS -> S0801LOS
	{ 0xcf8f5330, 0xd1442bcf }, // S0701MUS -> S0702MUS
	{ 0xd1442bcf, 0xddb4ebb1 }, // S0702MUS -> S0799MUS
	{ 0x0d511087, 0x0f05e926 }, // S0301CRM -> S0302CRM
	{ 0x0f05e926, 0x1b76a908 }, // S0302CRM -> S0399CRM
	{ 0x50a99ac9, 0x525e7368 }, // S0501PAS -> S0502PAS
	{ 0x525e7368, 0x50a99ac9 }, // S0502PAS -> S0501PAS
	{ 0xf275f57a, 0xf42ace19 }, // S1101BAM -> S1102BAM
	{ 0xf42ace19, 0x009b8dfb }, // S1102BAM -> S1199BAM
	{ 0x1e5492c9, 0x2c7a2b4a }, // S1601ISL -> S1699ISL		
};

#define MAX_PADS			4
#define LAYER2_KEY		0x21 // F for dinput8
#define HIDE_UI_KEY		0x3F // F5 for dinput8
#define VIRTUAL_L_TRIGGER	0x10000000	

typedef int (* MRCWIType)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int);
typedef int (* MCCType)(void *, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (* MGDGType)(void *, uint32_t);
typedef uint32_t (* VGHType)(void *, uint32_t, uint32_t, uint32_t);
typedef bool (* VGBType)(void *, uint32_t, uint32_t, uint32_t);
typedef uintptr_t (* LPAType)(void *, uint32_t, const char *, uint32_t);
typedef bool (* ICMType)(void *, uintptr_t, uintptr_t, uintptr_t);
typedef void (* BIType)(void *);
typedef void (* BFType)(void *);
typedef void (* BNMType)(void *);
typedef void (* BRMType)(void *);
typedef void * (*SRType)(void *, GameCharSlot *, void *, uint32_t, bool, bool, uintptr_t, uint8_t);
typedef uint32_t (*SRSType)(void *, void *, bool, bool);
typedef void (* CISLType)(void *obj, uint32_t costume, uint32_t player_index);
typedef void (* CRDSLType)(void *obj, uint32_t player_index);
typedef uint32_t (* CGDHLIHType)(void *obj, uint32_t player_index, uint32_t list_index); // returns hair hash
typedef uint32_t (* CGDHLSType)(void *obj, uint32_t player_index); // returns list size
typedef uint32_t (* CGDFMType)(uint32_t costume, uint32_t hair, uint32_t unk); // returns face hash
typedef uint32_t (* OGCRFCType)();
typedef void (* LSPTUType)(void *obj, uintptr_t unused, uint32_t pane_id, uint32_t r9, uint32_t sp_20, const char *text);
typedef void (* LSPTIIType)(void *, uint32_t, uint32_t, uint32_t, const char *, uint32_t, uint32_t);
typedef void * (*LSType)(void *, void *, uint32_t, uint32_t, void *);
typedef void (* SSPType)(void *, void *, void *, uint32_t);
typedef uint32_t (* GCSBGMType)();
typedef void (* LSDType)(void *, void *, uint16_t, bool);
typedef void * (* CLRIType)(void *pthis, void *unk2, uint8_t char_code, GameCharSlot2 *slot, uint8_t unk5);
typedef bool (* EventFuncType)(void *);
typedef void * (* GPMAOType)(uint32_t);
typedef void (* BR1Type)(uint8_t *, void *);
typedef void (* BR2Type)(void *, uint8_t);
typedef uint32_t (* GRFType)(void *, uint32_t, int);
typedef uint32_t (* GRHType)(void *, uint32_t, uint32_t, int);
typedef void * (* LCType)(void *);

typedef DWORD (* XIGSType)(DWORD, XINPUT_STATE *);
typedef HRESULT (* DI8GDSType)(void *, DWORD, LPVOID);

static MRCWIType model_request_character_with_id;
static MCCType model_character_completed;
static MGDGType model_get_default_glasses;
static VGHType var_get_hash;
static VGBType var_get_boolean;
static LPAType layout_play_anime;
static ICMType is_cached_model, is_cached_face;

static BIType battle_initialize;
static BFType battle_finalize;
static BNMType battle_next_match;
static BRMType battle_rematch;
static SRType select_randomness;
static SRSType select_randomness_stage;

static CISLType mode_closet_initialize_select_list;
static CRDSLType mode_closet_reset_detail_select_list;
static CGDHLIHType mode_closet_get_detail_hair_list_item_hash;
static CGDHLSType mode_closet_get_detail_hair_list_size;
static CGDFMType mode_closet_get_different_face_model;
static OGCRFCType mode_option_get_character_random_filter_count;

static LSPTUType layout_set_pane_text_utf8;
static LSPTIIType layout_set_pane_texture_ii;

static LSType load_stage;
static SSPType set_stage_params;
static GCSBGMType get_current_stage_bgm;

static LSDType load_slot_data;

CLRIType CharacterLoadRequestImpl_ctor;

static XIGSType XInputGetState_Orig;

static EventFuncType OnCharacterBreak, OnCharacterTransform;
static GPMAOType get_player_mao;

static BR1Type break_requirement1;
static BR2Type break_requirement2;

static GRFType get_related_face;
static GRHType get_related_hair;
static LCType load_costumes;

extern "C" PUBLIC int model_request_character_with_id_patched(void *pthis, uint32_t player_index, uint32_t chara_hash, uint32_t costume_hash, uint32_t face_hash, uint32_t hair_hash, uint32_t color, int hair_color);

static void **pmodel_object;
static void **pcloset_object;
static void **playout_object;
//static void **poption_object;
static void **pdamaged_character_event_object;
static bool disable_cache;
static int css_index = -1;
static int dss_index = -1;
static bool in_sss = false;
static bool listener_installed = false;
static bool in_battle;

static WORD previous_buttons[MAX_PADS+1]; // Will use the extra entry for keyboard

static std::unordered_map<uint64_t, Layer2Slot *> slots;
static Layer2Slot *player_slots[NUM_PLAYERS];
static Layer2Slot saved_random_slots[NUM_PLAYERS]{{0, 0, 0}, { 0, 0, 0} };
static RandomQueue rqueue;
static bool old_random_mode;
// Vars used by new random approach
static bool player1_random, player2_random;
static int next_player_index; // 0 or 1. -1 to disable
static int fight_num;
//

static bool got_battle_costume;

static std::unordered_map<uint32_t, Layer2StageSlot *> stage_slots;
static Layer2StageSlot *stage_slot;
static RandomStageQueue rqueue_stages;

static Mutex reqm_mutex;

static bool enable_random_costume_mods;
static bool enable_random_underwear;
static bool enable_random_hair;
static bool enable_random_glasses;
static float random_hair_chance;
static float random_glasses_chance;
static bool allow_random_hair_mods;
static bool enable_random_stage_mods;

static bool slot_info_in_css, slot_info_in_sss;

static std::vector<std::vector<uint32_t>> random_glasses_list;

static uint32_t survival_stage; // This is the one that will be patched into the exe, can only be a valid game stage
static uint32_t survival_stage_slot;
static uint32_t survival_stage_work;
static std::string survival_stage_mod_str; // only to be used by layer2_add_mod and layer2_finalization
static uint32_t *p_survival_stage; // only to be used by layer2_finalization and OnSurvivalStageLocated
static Layer2Mod *survival_stage_mod = nullptr; 

static std::unordered_map<uint32_t, uint32_t> defaults_faces, defaults_hairs, mapped_face;

static bool transform_p1, transform_p2;

static inline bool costume_exists(uint32_t hash)
{
	auto it = hash_to_costume.find(hash);
	return (it != hash_to_costume.end());
}

static inline  bool face_exists(uint32_t hash)
{
	auto it = hash_to_face.find(hash);
	return (it != hash_to_face.end());
}

static inline bool hair_exists(uint32_t hash)
{
	auto it = hash_to_hair.find(hash);
	return (it != hash_to_hair.end());
}

static inline bool glasses_exists(uint32_t hash)
{
	auto it = hash_to_glasses.find(hash);
	return (it != hash_to_glasses.end());
}

static inline bool stage_exists(uint32_t hash)
{
	auto it = hash_to_stage.find(hash);
	return (it != hash_to_stage.end());
}


static uint32_t get_stage_by_indexes(uint32_t idx1, uint32_t idx2)
{
	if (idx1 == 1)
	{
		if (idx2 == 1)
			return 0xfb6cc144; // S0101PIR  
		else if (idx2 == 2)
			return 0xfd2199e3; // S0102PIR
	}
	else if (idx1 == 8)
	{
		if (idx2 == 1)
			return 0x2b627f14; // S0801LOS
		else if (idx2 == 2)
			return 0x2d1757b3; // S0802LOS
	}
	else if (idx1 == 7)
	{
		if (idx2 == 1)
			return 0xcf8f5330; // S0701MUS
		else if (idx2 == 2)
			return 0xd1442bcf; // S0702MUS
	}
	else if (idx1 == 4 && idx2 == 1)
		return 0x17a511b2; // S0401CLS
	else if (idx1 == 9 && idx2 == 1)
		return 0x3c91a512; // S0901WAY
	else if (idx1 == 3)
	{
		if (idx2 == 1)
			return 0x0d511087; // S0301CRM
		else if (idx2 == 2)
			return 0x0f05e926;  // S0302CRM
	}
	else if (idx1 == 6 && idx2 == 1)
		return 0xff9c23fe; // S0601LBN
	else if (idx1 == 5)
	{
		if (idx2 == 1)
			return 0x50a99ac9; // S0501PAS
		else if (idx2 == 2)
			return 0x525e7368; // S0502PAS
	}
	else if (idx1 == 2 && idx2 == 1)
		return 0x4e2a96cd; // S0201BST
	else if (idx1 == 11)
	{
		if (idx2 == 1)
			return 0xf275f57a; // S1101BAM
		else if (idx2 == 2)
			return 0xf42ace19; // S1102BAM
	}
	else if (idx1 == 12 && idx2 == 1)
		return 0xe58a8038; // S1201PRO
	else if (idx1 == 13 && idx2 == 1)
		return 0xb8123b65; // S1301GYM
	else if (idx1 == 15 && idx2 == 1)
		return 0xaa830301; // S1501GRD
	else if (idx1 == 16 && idx2 == 1)
		return 0x1e5492c9; // S1601ISL
	
	return 0;
}

static inline bool is_boss_stage(uint32_t hash)
{
	return (hash == 0x04b88a2d);
}

static inline bool is_bossl_stage(uint32_t hash)
{
	return (hash == 0x8668d961);
}

static inline bool is_colnight_stage(uint32_t hash)
{
	return (hash == 0x0aa42598);
}	

static inline bool is_rrnight_stage(uint32_t hash)
{
	return (hash == 0x73f4de08);
}

static inline bool is_throwa_stage(uint32_t hash)
{
	return (hash == 0x076a493b);
}

static inline bool is_special_stage(uint32_t hash)
{
	return (is_boss_stage(hash) || is_bossl_stage(hash) || is_colnight_stage(hash) || is_rrnight_stage(hash) || is_throwa_stage(hash));
}

static inline uint32_t resolve_work_stage(uint32_t hash)
{
	if (is_boss_stage(hash) || is_bossl_stage(hash))
		return 0xf42ace19; // Hidden Forest
	
	if (is_colnight_stage(hash))
		return 0x17a511b2; // Colosseum
	
	if (is_rrnight_stage(hash))
		return 0x3c91a512; // Road Rage
	
	if (is_throwa_stage(hash))
		return 0x4e2a96cd; // Throwdown
	
	return hash;
}

__attribute__ ((unused)) static std::string get_costume_name(uint32_t costume) 
{
	auto it = hash_to_costume.find(costume);
	if (it != hash_to_costume.end())
	{
		return it->second;
	}
	
	return Utils::UnsignedToHexString(costume, true);
}

__attribute__ ((unused)) static std::string get_hair_name(uint32_t hair) 
{
	auto it = hash_to_hair.find(hair);
	if (it != hash_to_hair.end())
	{
		return it->second;
	}
	
	return Utils::UnsignedToHexString(hair, true);
}

__attribute__ ((unused)) static std::string get_face_name(uint32_t face) 
{
	auto it = hash_to_face.find(face);
	if (it != hash_to_face.end())
	{
		return it->second;
	}
	
	return Utils::UnsignedToHexString(face, true);
}

__attribute__ ((unused)) static std::string get_glasses_name(uint32_t glasses) 
{
	auto it = hash_to_glasses.find(glasses);
	if (it != hash_to_glasses.end())
	{
		return it->second;
	}
	
	return Utils::UnsignedToHexString(glasses, true);
}

void show_slot_info(Layer2Slot *slot)
{
	//DPRINTF("show slot info %d %d\n", css_index, dss_index);
	
	if (!playout_object || !*playout_object || !slot_info_in_css)
		return;
	
	std::string text;
	
	if (css_index >= 0)
	{
		std::string cos_text;
		std::string work_text;
		std::string mod_text;
		
		auto it = hash_to_costume.find(slot->costume);
		if (it != hash_to_costume.end())
		{
			cos_text = it->second;			
		}
		else
		{
			cos_text = Utils::UnsignedToHexString(slot->costume, true);
		}
		
		uint32_t work = slot->GetWorkCostume();			
		if (slot->costume != work)
		{
			auto it2 = hash_to_costume.find(work);
			if (it2  != hash_to_costume.end())
			{
				work_text = it2->second;	
			}
			else
			{
				work_text = Utils::UnsignedToHexString(work, true);
			}
		}
		
		Layer2Mod *mod = slot->GetCurrentMod();
		if (mod)
		{
			mod_text = mod->friendly_name;
		}
		else
		{
			mod_text = "Vanilla";
		}
		
		text = "Slot: " + cos_text + " ";
		
		if (work_text.length() > 0)
			text += " (Work: " + work_text + ") ";
		
		text += "Mod: " + mod_text;
	}
	else if (dss_index >= 0)
	{
		std::string hair_text;
		std::string work_text;
		std::string mod_text;
		
		auto it = hash_to_hair.find(slot->current_hair);
		if (it != hash_to_hair.end())
		{
			hair_text = it->second;			
		}
		else
		{
			hair_text = Utils::UnsignedToHexString(slot->current_hair, true);
		}
		
		uint32_t work = slot->GetWorkHair();	
		
		if (slot->current_hair != work)
		{
			//DPRINTF("work = %08x, slot = %08x\n", work,slot->current_hair);
			auto it2 = hash_to_hair.find(work);
			if (it2  != hash_to_hair.end())
			{
				work_text = it2->second;	
			}
			else
			{
				work_text = Utils::UnsignedToHexString(work, true);
			}
		}
		
		Layer2Mod *mod = slot->GetCurrentHairMod();
		if (mod)
		{
			mod_text = mod->friendly_name;
		}
		else
		{
			mod_text = "Vanilla";
		}
		
		text = "Slot: " + hair_text + " ";
		
		if (work_text.length() > 0)
			text += " (Work: " + work_text + ") ";
		
		text += "Mod: " + mod_text;
	}
	
	if (text.length() > 0)
	{
		layout_set_pane_text_utf8(*playout_object, 0, 0xF121F112, 0, 4, text.c_str());
	}
}

void show_stage_slot_info(Layer2StageSlot *slot)
{
	if (!playout_object || !*playout_object || !slot_info_in_sss)
		return;
	
	std::string text;
	
	if (in_sss)
	{
		std::string stg_text;
		std::string work_text;
		std::string mod_text;
		
		auto it = hash_to_stage.find(slot->stage);
		if (it != hash_to_stage.end())
		{
			stg_text = it->second;			
		}
		else
		{
			stg_text = Utils::UnsignedToHexString(slot->stage, true);
		}
		
		uint32_t work = slot->GetWorkStage();			
		if (slot->stage != work)
		{
			auto it2 = hash_to_stage.find(work);
			if (it2  != hash_to_stage.end())
			{
				work_text = it2->second;	
			}
			else
			{
				work_text = Utils::UnsignedToHexString(work, true);
			}
		}
		
		Layer2Mod *mod = slot->GetCurrentMod();
		if (mod)
		{
			mod_text = mod->friendly_name;
		}
		else
		{
			mod_text = "Vanilla";
		}
		
		text = "Slot: " + stg_text + " ";
		
		if (work_text.length() > 0)
			text += " (Work: " + work_text + ") ";
		
		text += "Mod: " + mod_text;
	}
	
	if (text.length() > 0)
	{
		layout_set_pane_text_utf8(*playout_object, 0, 0xF121F112, 0, 4, text.c_str());
	}
}

void on_mod_button_pressed(bool direction_up)
{
	int update_index = -1;
	Layer2Slot *slot = nullptr;
	
	static uint64_t next_allowed = 0;
	uint64_t tick = GetTickCount64();
	
	if (tick < next_allowed)
		return;
	
	next_allowed = tick + 500;
	
	if (css_index >= 0)
	{	
		slot = player_slots[css_index];
		if (!slot)
			return;
		
		if (slot->Advance(direction_up))
		{
			update_index = css_index;
		}
	}
	else if (dss_index >= 0)
	{
		slot = player_slots[dss_index];
		if (!slot)
			return;
		
		if (slot->AdvanceHair(direction_up))
		{
			update_index = dss_index;
		}
	}
	else if (in_sss)
	{
		if (!stage_slot)
			return;
		
		if (stage_slot->Advance(direction_up))
		{
			show_stage_slot_info(stage_slot);
		}
	}
	
	if (update_index >= 0)
	{
		//DPRINTF("Will reload model: mi=%d hi=%d (player = %d)\n", slot->mod_index, slot->hair_mod_index, update_index);
		
		/*rdbemu_set_layer2_mode(update_index);
		
		rdbemu_set_layer2(slot->GetCurrentMod(), update_index);
		rdbemu_set_layer2_hair(slot->GetCurrentHairMod(), update_index);		
		
		MutexLocker lock(&reqm_mutex);
		
		disable_cache = true;
		show_slot_info(slot);*/
		
		if (slot->current_hair_color != -1)
		{
			DPRINTF("Notice: going to issue call with hair_color != -1 (%d)\n", slot->current_hair_color);
		}
		
		model_request_character_with_id_patched(*pmodel_object, update_index | 0x80000000, slot->chara_hash, slot->costume, slot->current_face, slot->current_hair, slot->current_color, slot->current_hair_color);
		//disable_cache = false;
	}
}

static DWORD XinputGetState_Patched(DWORD dwUserIndex, XINPUT_STATE *pState)
{
	DWORD ret = XInputGetState_Orig(dwUserIndex, pState);
	
	if (!listener_installed)
		return ret;
	
	if (ret == ERROR_SUCCESS && dwUserIndex >= 0 && dwUserIndex < MAX_PADS)
	{
		DWORD buttons = pState->Gamepad.wButtons;
		
		if (pState->Gamepad.bLeftTrigger > 0x80)
			buttons |= VIRTUAL_L_TRIGGER;
		else
			buttons &= ~VIRTUAL_L_TRIGGER;
				
		if ((buttons & ~previous_buttons[dwUserIndex]) & XINPUT_GAMEPAD_BACK)
		{
			//DPRINTF("Back button pressed!\n");	
			on_mod_button_pressed(true);			
		}
		else if ((buttons & ~previous_buttons[dwUserIndex]) & VIRTUAL_L_TRIGGER)
		{
			on_mod_button_pressed(false);
		}
		
		previous_buttons[dwUserIndex] = buttons;
	}
	
	return ret;
}

static HRESULT DirectInputDevice8_GetDeviceState_Patched(void *obj, DWORD outDataLen, uint8_t *outData)
{
	uintptr_t *vtable = (uintptr_t *) *(uintptr_t *)obj;
	DI8GDSType DirectInputDevice8_GetDeviceState = (DI8GDSType) vtable[9];
		
	HRESULT ret = DirectInputDevice8_GetDeviceState(obj, outDataLen, outData);
	
	if (in_battle)
	{
		if (ret == S_OK)
		{
			DWORD buttons = (outData[HIDE_UI_KEY] != 0);
					
			if ((buttons & ~previous_buttons[MAX_PADS]))
			{
				//DPRINTF("You pressed the F5 key. Congratulations.\n");
				toggle_battle_hud();				
			}
			
			previous_buttons[MAX_PADS] = buttons;
		}
	}
	
	if (!listener_installed)
		return ret;
	
	if (ret == S_OK)
	{
		DWORD buttons = (outData[LAYER2_KEY] != 0);
				
		if ((buttons & ~previous_buttons[MAX_PADS]))
		{
			//DPRINTF("You pressed the F key. Congratulations.\n");
			on_mod_button_pressed(true);
		}
		
		previous_buttons[MAX_PADS] = buttons;
	}
	
	return ret;
}

static void install_listener()
{
	if (listener_installed)
		return;
	
	memset(previous_buttons, 0, sizeof(previous_buttons));	
	listener_installed = true;
}

// We will be calling this before battles so to make sure that there is not overhead (which is probably unnoticeable, but just in case)
static void uninstall_listener()
{
	if (!listener_installed)
		return;
	
	listener_installed = false;
}

void layer2_init()
{
	int random_hair_probability, random_glasses_probability;
	
	ini.GetBooleanValue("Random", "enable_random_costume_mods", &enable_random_costume_mods, true);	
	ini.GetBooleanValue("Random", "enable_random_underwear", &enable_random_underwear, false);
	ini.GetBooleanValue("Random", "enable_random_hair", &enable_random_hair, false);
	ini.GetBooleanValue("Random", "enable_random_glasses", &enable_random_glasses, false);
	ini.GetBooleanValue("Random", "enable_random_stage_mods", &enable_random_stage_mods, true);	
	ini.GetIntegerValue("RandomHair", "probability", &random_hair_probability, 35);
	ini.GetBooleanValue("RandomHair", "allow_mods", &allow_random_hair_mods, true);
	ini.GetIntegerValue("RandomGlasses", "probability", &random_glasses_probability, 25);
	
	ini.GetBooleanValue("Misc", "slot_info_in_css", &slot_info_in_css, true);
	ini.GetBooleanValue("Misc", "slot_info_in_sss", &slot_info_in_sss, true);
	
	ini.GetBooleanValue("Debug", "old_random_mode", &old_random_mode, false);
	
	if (random_hair_probability < 0)
		random_hair_probability = 0;
	else if (random_hair_probability > 100)
		random_hair_probability = 100;
	
	random_hair_chance = ((float)random_hair_probability) / 100.0f;	
	//DPRINTF("random_hair_chance = %f\n", random_hair_chance);
	
	if (random_glasses_probability < 0)
		random_glasses_probability = 0;
	else if (random_glasses_probability > 100)
		random_glasses_probability = 100;
	
	random_glasses_chance = ((float)random_glasses_probability) / 100.0f;	
	//DPRINTF("random_glasses_chance = %f\n", random_glasses_chance);
	
	if (enable_random_glasses)
	{
		random_glasses_list.resize(CHAR_NUM);
		
		for (uint8_t i = CHAR_CMN+1; i < CHAR_NUM; i++)
		{
			std::string ch = character_id_to_prefix(i);
			std::vector<std::string> values;
			
			if (ini.GetMultipleStringsValues("RandomGlasses", ch, values))
			{
				for (const std::string &str : values)
				{
					uint32_t glasses = hash_func(str);
					
					if (glasses_exists(glasses))
					{
						random_glasses_list[i].push_back(glasses);
					}
					else
					{
						DPRINTF("Warning: glasses \"%s\" specified in the ini is not recognized, it will be omitted.\n", str.c_str());
					}
				}
			}
		}
	}
	
	for (auto &it : hash_to_costume)
	{
		std::string chara = it.second.substr(0, 3);
		uint32_t chara_hash = hash_func(chara);
		
		Layer2Slot *l2s_0 = new Layer2Slot(chara_hash, it.first, 0);	
		Layer2Slot *l2s_1 = new Layer2Slot(chara_hash, it.first, 1);	
		
		slots[it.first] = l2s_0;
		slots[it.first | (1ULL << 32)] = l2s_1;
	}
	
	for (auto &it : hash_to_stage)
	{
		Layer2StageSlot *slot = new Layer2StageSlot(it.first);
		stage_slots[it.first] = slot;
	}
}

static bool get_hash_value(IniFile &cos, const std::string &mod_name, const std::string &section, const std::string &name, uint32_t *ret, uint32_t backup, bool mandatory, uint32_t *powner=nullptr)
{
	std::string value;
	
	if (!cos.GetStringValue(section, name, value))
	{
		if (mandatory)
		{
			FTPRINTF(true, "Error in mod \"%s\".\n\nError in %s section: slot field is mandatory.\n", mod_name.c_str(), section.c_str());	
			return false;
		}
		else
		{
			*ret = backup;	
			return true;
		}
	}
	
	value = Utils::ToUpperCase(value);
	uint32_t hash = hash_func(value);
	
	/*if (name == "Costume" && !costume_exists(hash))
	{
		FTPRINTF(true, "Error in mod \"%s\".\n\nError in Costume section: \"%s\" is not a valid costume.\n", mod_name.c_str(), value.c_str());
		return false;
	}
	else if (name == "Hair" && !hair_exists(hash))
	{
		FTPRINTF(true, "Error in mod \"%s\".\n\nError in Hair section: \"%s\" is not a valid hair.\n", mod_name.c_str(), value.c_str());
		return false;
	}
	else if (name == "Face" && !face_exists(hash))
	{
		FTPRINTF(true, "Error in mod \"%s\".\n\nError in Face section: \"%s\" is not a valid face.\n", mod_name.c_str(), value.c_str());
		return false;
	}*/
	
	if (powner)
	{
		*powner = hash_func(value.substr(0, 3));
	}
	
	*ret = hash;		
	return true;
}

static std::vector<Layer2Mod *> temp_default_hair_mods; // Used temporary, for layer2_finalization to resolve mods that have another mod in default hair
static std::unordered_map<std::string, Layer2Mod *> temp_hair_mods_map; // Used temporary, for layer2_finalization to resolve mods that have another mod in default hair

bool layer2_add_mod(const std::string &name, const std::string &ini_path, std::unordered_map<uint32_t, VirtualRdbEntry> *cfiles, std::unordered_map<uint32_t, VirtualRdbEntry> *mfiles,
					std::unordered_map<uint32_t, VirtualRdbEntry> *kfiles, std::unordered_map<uint32_t, VirtualRdbEntry> *ffiles, std::unordered_map<uint32_t, VirtualRdbEntry> *rfiles)
{
	IniFile m_ini;
	
	if (!m_ini.LoadFromFile(ini_path))
		return false;
	
	Layer2Mod *mod = new Layer2Mod(name);
	
	std::string type;
	if (m_ini.GetStringValue("General", "type", type))
	{
		type = Utils::ToLowerCase(type);
		if (type == "costume")
		{
			mod->type = MOD_TYPE_COSTUME; // Not really needed, since constructor already defaults to this
		}
		else if (type == "hair")
		{
			mod->type = MOD_TYPE_HAIR;
		}
		else if (type == "stage")
		{
			mod->type = MOD_TYPE_STAGE;
		}
		else
		{
			FTPRINTF(true, "Error in mod \"%s\".\ntype \"%s\" is not recognized.\n", name.c_str(), type.c_str());
			delete mod;
			return false;
		}
	}

	if (mod->type == MOD_TYPE_COSTUME)
	{
		if (!m_ini.SectionExists("Costume"))
		{
			FTPRINTF(true, "Error in mod \"%s\".\n\nCostume section is mandatory for costume mods.\n", name.c_str());
			delete mod;
			return false;
		}
		
		if (!get_hash_value(m_ini, name, "Costume", "slot", &mod->slot_costume, 0, true, &mod->chara_owner))
		{
			delete mod;
			return false;
		}
		
		if (!get_hash_value(m_ini, name, "Costume", "work", &mod->work_costume, mod->slot_costume, false))
		{
			delete mod;
			return false;
		}
		
		if (mod->work_costume == 0x73f15cc3)
		{
			mod->is_random = true;
			mod->work_costume = mod->slot_costume;
		}
		
		if (!mod->is_random)
		{
			m_ini.GetBooleanValue("General", "transform", &mod->is_transform, false);
		}
		
		std::string glasses;
		if (m_ini.GetStringValue("Costume", "glasses", glasses))
		{
			mod->has_default_glasses = true;
			
			glasses = Utils::ToUpperCase(glasses);
			if (glasses == "NONE")
			{
				mod->default_glasses = 0;
			}
			else
			{
				mod->default_glasses = hash_func(glasses);
				if (!glasses_exists(mod->default_glasses))
				{
					DPRINTF("Warning: mod \"%s\": glasses \"%s\" are not recognized. This could cause a crash if glasses don't really exist.\n", mod->name.c_str(), glasses.c_str());	
					//return false;
				}
			}
		}
	}
	
	if (mod->type != MOD_TYPE_STAGE && m_ini.SectionExists("Hair"))
	{
		if (!get_hash_value(m_ini, name, "Hair", "slot", &mod->slot_hair, 0, true, (mod->type == MOD_TYPE_HAIR) ? &mod->chara_owner : nullptr))
		{
			delete mod;
			return false;
		}
	
		if (!get_hash_value(m_ini, name, "Hair", "work", &mod->work_hair, mod->slot_hair, false))
		{
			delete mod;
			return false;
		}
	}
	else if (mod->type == MOD_TYPE_HAIR)
	{
		FTPRINTF(true, "Error in mod \"%s\".\n\nHair section is mandatory for hair mods.\n", name.c_str());
		delete mod;
		return false;
	}
	
	if (mod->type != MOD_TYPE_STAGE && m_ini.SectionExists("Face"))
	{
		if (!get_hash_value(m_ini, name, "Face", "slot", &mod->slot_face, 0, true))
		{
			delete mod;
			return false;
		}
	
		if (!get_hash_value(m_ini, name, "Face", "work", &mod->work_face, mod->slot_face, false))
		{
			delete mod;
			return false;
		}
	}
	
	if (mod->type == MOD_TYPE_STAGE)
	{
		std::string bgm;
		
		if (!get_hash_value(m_ini, name, "Stage", "slot", &mod->slot_stage, 0, true))
		{
			delete mod;
			return false;
		}
		
		if (!get_hash_value(m_ini, name, "Stage", "work", &mod->work_stage, mod->slot_stage, false))
		{
			delete mod;
			return false;
		}
		
		m_ini.GetBooleanValue("Stage", "disable_npc", &mod->disable_npc, false);
		if (m_ini.GetStringValue("Stage", "bgm", bgm))
		{
			bgm = Utils::ToLowerCase(bgm);
			mod->bgm = hash_func(bgm);
		}
	}
	
	if (mod->type == MOD_TYPE_COSTUME)
	{
		std::string value;
		
		mod->default_hair = 0; // Not really needed, done in constructor	
		mod->default_hair_mod = nullptr; // Not really needed, done in constructor		
		
		if (m_ini.GetStringValue("General", "default_hair", value))
		{
			uint32_t hash = hash_func(Utils::ToUpperCase(value));
			if (hair_exists(hash))
			{
				mod->default_hair = hash;
			}
			else
			{
				// It will be resolved later in layer2 finalization
				mod->default_hair_mod_name = value;
				temp_default_hair_mods.push_back(mod);
			}
		}
	}
	else if (mod->type == MOD_TYPE_HAIR)
	{
		temp_hair_mods_map[Utils::ToLowerCase(mod->name)] = mod;
	}
	
	mod->character_files = cfiles;
	mod->material_files = mfiles;
	mod->kids_files = kfiles;
	mod->field4_files = ffiles;
	mod->rrp_files = rfiles;
	
	if (mod->type == MOD_TYPE_COSTUME)
	{	
		auto it = slots.find(mod->slot_costume);
		auto it2 = slots.find(mod->slot_costume | (1ULL << 32));
		if (it != slots.end() && it2 != slots.end())
		{
			it->second->mods.push_back(mod);
			it2->second->mods.push_back(mod);
			DPRINTF("Costume mod \"%s\" successfully loaded.\n", name.c_str());
		}
		else
		{
			FTPRINTF(false, "Error in mod %s, slot 0x%08x is not recognized.\n", name.c_str(), mod->slot_costume);
			return false;
		}
	}
	else if (mod->type == MOD_TYPE_HAIR)
	{
		// We need to insert to all slots of this character
		int count = 0;
		
		for (auto &it : slots)
		{
			if (it.second->chara_owner == mod->chara_owner)
			{
				Layer2HairSlot *hslot;
				auto it_h = it.second->hair_map.find(mod->slot_hair);
				if (it_h == it.second->hair_map.end())
				{
					hslot = new Layer2HairSlot();
					it.second->hair_map[mod->slot_hair] = hslot;
				}
				else
				{
					hslot = it_h->second;
				}
				
				hslot->mods.push_back(mod);
				count++;
			}
		}
		
		//DPRINTF("Hair added to %d slots.\n", count);
		DPRINTF("Hair mod \"%s\" successfully loaded.\n", name.c_str());
	}
	else if (mod->type == MOD_TYPE_STAGE)
	{
		auto it = stage_slots.find(mod->slot_stage);
		if (it != stage_slots.end())
		{
			if (!survival_stage_mod && survival_stage_mod_str.length() > 0)
			{
				if (Utils::ToLowerCase(survival_stage_mod_str) == Utils::ToLowerCase(mod->name))
				{
					survival_stage_mod = mod;
				}
			}
			
			it->second->mods.push_back(mod);
			DPRINTF("Stage mod \"%s\" successfully loaded.\n", name.c_str());
		}
		else
		{
			FTPRINTF(false, "Error in mod %s, slot 0x%08x is not recognized.\n", name.c_str(), mod->slot_stage);
			return false;
		}
	}
	
	return true;
}

void layer2_finalization()
{
	for (Layer2Mod *mod : temp_default_hair_mods)
	{
		std::string search_name = Utils::ToLowerCase(mod->default_hair_mod_name);
		
		if (mod->name != mod->friendly_name)
		{
			size_t pos = mod->name.rfind('/');
			if (pos != std::string::npos && (pos != mod->name.length()-1)) // Should always evaluate to true
			{
				search_name = Utils::ToLowerCase(mod->name.substr(0, pos+1)) + search_name;
			}
		}
		
		auto it = temp_hair_mods_map.find(search_name);
		if (it != temp_hair_mods_map.end())
		{
			Layer2Mod *hmod = it->second;
			
			if (mod->chara_owner != hmod->chara_owner)
			{
				DPRINTF("Mod \"%s\" cannot be used as default hair for mod \"%s\" because they are for different characters.\n", hmod->name.c_str(), mod->name.c_str());
				continue;
			}
			
			mod->default_hair_mod = hmod;
			mod->default_hair = hmod->slot_hair;
		}
		else
		{
			DPRINTF("Warning: in mod \"%s\", default_hair \"%s\" couldn't be resolved (mod not found)\n", mod->name.c_str(), mod->default_hair_mod_name.c_str());
		}
	}
	
	// Remove from emmory uneeded things
	temp_default_hair_mods.clear();
	
	// Handle survival stage now
	if (survival_stage_mod)
	{
		survival_stage = survival_stage_mod->work_stage;
		survival_stage_slot = survival_stage_mod->slot_stage;
	}
	else
	{
		if (survival_stage_mod_str.length() > 0)
		{
			DPRINTF("Warning: Misc:survival_stage_mod: mod %s not found, doesn't have an .ini or is not a stage mod.\n", survival_stage_mod_str.c_str()); 
		}
	}
	
	if (survival_stage != 0)
	{	
		survival_stage_work = survival_stage;
		survival_stage = resolve_work_stage(survival_stage);
		
		if (survival_stage_slot == 0)
			survival_stage_slot = survival_stage;
		
		if (p_survival_stage)
			PatchUtils::Write32(p_survival_stage, survival_stage);
	}
}

void battle_select_end()
{
	rdbemu_set_layer2_mode(-1);
	
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		Layer2Slot *slot = player_slots[i];
		Layer2Mod *mod = nullptr;
		Layer2Mod *hair_mod = nullptr;
		
		if (slot)
		{
			mod = slot->GetCurrentMod();
			hair_mod = slot->GetCurrentHairMod();
		}
		
		rdbemu_set_layer2(mod, i);
		rdbemu_set_layer2_hair(hair_mod, i);
		
		//player_slots[i] = nullptr; No! Because of randomness, this is now moved to battle_finalize_patched
	}
}

static void process_randomness(Layer2Slot *rslot, uint32_t player_index)
{
	Layer2Slot *slot = nullptr;
	Layer2Mod *mod = nullptr;
	Layer2Mod *hair_mod = nullptr;
		
	auto it = slots.find(rslot->costume | ((uint64_t)player_index << 32));
	
	if (it != slots.end())
	{
		slot = it->second;
		*slot = *rslot;
		
		mod = slot->GetCurrentMod();
		hair_mod = slot->GetCurrentHairMod();
	}
	
	rdbemu_set_layer2(mod, player_index);
	rdbemu_set_layer2_hair(hair_mod, player_index);
}

static void process_randomness_stage(Layer2StageSlot *rslot)
{
	Layer2StageSlot *slot = nullptr;
	Layer2Mod *mod = nullptr;
	
	auto it = stage_slots.find(rslot->stage);
	if (it != stage_slots.end())
	{
		slot = it->second;
		*slot = *rslot;
		
		mod = slot->GetCurrentMod();
		
		// Old code, not longer needed
		/* if (mod && is_special_stage(slot->GetWorkStage()))
		{
			activate_special_next = true;			
			
			if (is_bossl_stage(slot->GetWorkStage()))
			{
				activate_special_param = 1;
			}
		}*/
	}
	
	rdbemu_set_layer2_stage(mod);
}

static size_t get_hair_list(uint32_t costume, uint32_t player_index, std::vector<uint32_t> &hair_list, bool init=true, bool clear=true)
{
	// Imitation of how the game gets the list of hair available for a specific costume	
	void *closet = *pcloset_object;
	if (!closet)
		return 0;
	
	if (init)	
		mode_closet_initialize_select_list(closet, costume, player_index);
	
	uint32_t num = mode_closet_get_detail_hair_list_size(closet, player_index);
	
	for (uint32_t i = 0; i < num; i++)
	{
		hair_list.push_back(mode_closet_get_detail_hair_list_item_hash(closet, player_index, i));
	}
	
	if (clear)
		mode_closet_reset_detail_select_list(closet, player_index);
	
	return (size_t)num;
}

// Random flag: if it is was the user who selected the randomness (as opossed to being selected automatically by certain modes)
// char_id: randomness only applies to this char. Game uses 0xFF all the time to include all characters.
static void *select_randomness_common(void *pthis, GameCharSlot *game_slot, void *random_input_data, uint32_t player_index, bool random_flag, bool cpu_random_flag, uintptr_t unused, uint8_t char_id)
{
	Layer2Slot *current_slot = player_slots[player_index];
	bool single_char_random = false;
	
	if (current_slot && current_slot->IsRandom())
	{
		char_id = character_hash_to_id(current_slot->chara_hash);
		saved_random_slots[player_index] = *current_slot;
		single_char_random = true;
	}
	else if (!current_slot && saved_random_slots[player_index].chara_owner != 0)
	{
		if (saved_random_slots[player_index].IsRandom())
		{
			char_id = character_hash_to_id(saved_random_slots[player_index].chara_hash);
			single_char_random = true;
		}
	}
	
	void *ret = select_randomness(pthis, game_slot, random_input_data, player_index, random_flag, cpu_random_flag, unused, char_id);
	
	if (player_index >= NUM_PLAYERS || rqueue.mode < 0)
		return ret;
	
	//DPRINTF("randomness_common %d\n", player_index);
	
	Layer2Slot *other_player = nullptr;
	Layer2Slot *slot = nullptr;
	Layer2Slot rslot(0, 0xDEADC0DE, player_index);
	
	if (current_slot)
	{
		current_slot->mod_index = -1;
		current_slot->SetCurrentHairModIndex(-1);
		player_slots[player_index] = nullptr;
	}
	
	if (player_index == 0)
		other_player = player_slots[1];
	else
		other_player = player_slots[0];
	
	if (!other_player && player_index == 1)
	{
		if (rqueue.mode == MODE_VS || rqueue.mode == MODE_FREE_TRAINING)
		{
			other_player = rqueue.GetNextSlot(0);
		}
		else if (rqueue.mode == MODE_ARCADE || rqueue.mode == MODE_SURVIVAL)
		{
			other_player = rqueue.GetSlotAt(0, 0);
		}
	}
	
	bool use_randomization = true;
	bool use_hair_randomization = false;
	
	auto it = slots.find(game_slot->costume_hash | ((uint64_t)player_index << 32));
	if (it != slots.end())
	{
		slot = it->second;
	}
	
	if (slot)
	{
		rslot = *slot;
	}
	
	if (!slot || slot->mods.size() == 0 || !enable_random_costume_mods)
	{
		use_randomization = false;
	}
	else if (other_player && (other_player->costume == game_slot->costume_hash || other_player->GetWorkCostume() == game_slot->costume_hash))
	{
		use_randomization = false;
	}
	
	if (enable_random_hair)
	{
		use_hair_randomization = (Utils::RandomProbability() <= random_hair_chance);
		
		if (use_hair_randomization && other_player)
		{
			if (other_player->current_hair == game_slot->hair_hash || other_player->GetWorkHair() == game_slot->hair_hash)
				use_hair_randomization = false;
		}

		if (use_hair_randomization)
		{
			std::vector<uint32_t> hair_list;							
			get_hair_list(game_slot->costume_hash, player_index, hair_list);
			
			if (hair_list.size() > 0)
			{
				size_t rnd = (size_t)Utils::RandomInt(0, hair_list.size()); // >= 0 && < hair_list.size
				
				if (rnd >= hair_list.size()) // This is just in case I change the implementation of RandomInt...
					rnd = 0;
					
				if (hair_list[rnd] != game_slot->hair_hash)
				{
					uint32_t prev_face = game_slot->face_hash; 
					game_slot->face_hash = mode_closet_get_different_face_model(game_slot->costume_hash, hair_list[rnd], 0);
					
					if (prev_face != game_slot->face_hash)
					{
						// We will only be here in few cases, LIS is the most likley character that has a different face associated to a costume/hair
						//DPRINTF("Face changed from 0x%08x to 0x%08x (costume=0x%08x, hair=0x%08x)\n", prev_face, game_slot->face_hash, game_slot->costume_hash, hair_list[rnd]);
					}
				}
					
				game_slot->hair_hash = hair_list[rnd];
			}
			else
			{
				use_hair_randomization = false;
			}
		}
	}
	
	rslot.current_face = game_slot->face_hash;
	rslot.current_hair = game_slot->hair_hash;
	rslot.current_color = game_slot->color;
	rslot.current_hair_color = game_slot->hair_color;
	rslot.mod_index = -1;
	rslot.SetCurrentHairModIndex(-1);
		
	if (use_randomization)
	{
		int rnd = (int)Utils::RandomInt(0, slot->mods.size()+1); // >= 0, && <= slot->mods.size
	
		if (rnd >= (int)slot->mods.size())
		{
			rnd = -1; // Vanilla
		}
		
		rslot.mod_index = rnd;
	}
	
	if (use_hair_randomization && allow_random_hair_mods && slot)
	{
		size_t num_h_mods = rslot.GetNumHairModsForCurrentHair();		
		if (num_h_mods > 0)
		{		
			int rnd = (int)Utils::RandomInt(0, num_h_mods+1); // >= 0 && <= num_h_mods
			
			if (rnd >= (int)num_h_mods)
			{
				rnd = -1; // Vanilla
			}
						
			rslot.SetCurrentHairModIndex(rnd);
		}		
	}
	
	if (slot)
	{
		game_slot->costume_hash = rslot.GetWorkCostume();
		game_slot->face_hash = rslot.GetWorkFace();
		game_slot->hair_hash = rslot.GetWorkHair();
	}
	
	if (enable_random_underwear)
	{
		game_slot->underwear = Utils::RandomInt(0, 4); // should generate >= 0 && <= 3
		if (game_slot->underwear >= 4) // Failsafe just in case I screw RandomInt implementation in the future
			game_slot->underwear = 0;
	}
	
	if (enable_random_glasses && game_slot->char_id < CHAR_NUM && Utils::RandomProbability() <= random_glasses_chance)
	{
		std::vector<uint32_t> other_list;
		std::vector<uint32_t> *plist;
		
		if (game_slot->glasses_hash == 0)
		{
			plist = &random_glasses_list[game_slot->char_id];
		}
		else
		{
			other_list = random_glasses_list[game_slot->char_id];
			other_list.push_back(0); // Insert no glasses
			// Remove current glasses
			for (size_t i = 0; i < other_list.size(); i++)
			{
				if (other_list[i] == game_slot->glasses_hash)
				{
					other_list.erase(other_list.begin() + i);
					i--;
				}
			}
			
			plist = &other_list;
		}
		
		if (plist->size() > 0)
		{		
			int rnd = Utils::RandomInt(0, plist->size()); 
			if (rnd >= (int)plist->size()) // Failsafe just in case I screw RandomInt implementation in the future
				rnd = 0;
				
			game_slot->glasses_hash = (*plist)[rnd];
		}
	}
	
	if (rqueue.mode == MODE_VS || rqueue.mode == MODE_FREE_TRAINING)
	{
		rqueue.GetQueue(player_index).push_back(rslot);
		
		if (rqueue.QueueSize(player_index) == 1)
		{
			if (old_random_mode)
			{
				process_randomness(&rslot, player_index);
			}
			else
			{
				if (player_index == 0)
					player1_random = true;
				else
					player2_random = true;
			}
		}
	}
	else if (rqueue.mode == MODE_ARCADE)
	{
		rqueue.GetQueue(player_index).push_back(rslot);
		
		if (rqueue.QueueSize(player_index) == 1)
		{
			if (old_random_mode)
			{
				process_randomness(&rslot, player_index);
			}
			else
			{
				if (player_index == 0)
					player1_random = true;
				
				player2_random = true;
			}
		}
	}
	else if (rqueue.mode == MODE_SURVIVAL)
	{
		rqueue.GetQueue(player_index).push_back(rslot);
		
		if (old_random_mode)
		{		
			if (player_index == 0)
			{
				process_randomness(&rslot, player_index);
			}
			else
			{
				if (rqueue.QueueSize(player_index) == 2)
				{
					// We will only process first cpu opponent if it doesn't collide with second
					Layer2Slot *prev = rqueue.GetSlotAt(player_index, 0);
					if (prev && prev->chara_owner != rslot.chara_owner)
					{
						process_randomness(prev, player_index);
					}
				}
			}
		}
		else
		{
			if (rqueue.QueueSize(player_index) == 1)
			{
				if (player_index == 0)
					player1_random = true;
				
				player2_random = true;
			}
		}
	}
	
	if ((player_index == 1 || rqueue.mode == MODE_VS) && rqueue.QueueSize(player_index) > 1)
	{
		// Atm, don't allow two random characters to be same in a row. This is to hide a current problem with game caching files...
		Layer2Slot *r_prev = rqueue.GetSlotAt(player_index, rqueue.QueueSize(player_index)-2);
		if (r_prev && r_prev->chara_owner == rslot.chara_owner && !single_char_random)
		{
			// But if this is a user-selected random, then the randomness only applies to the random filter count. And if only one char, we would enter infinite loop, so check this case
			if (random_flag && mode_option_get_character_random_filter_count() <= 1)
			{
				return ret;
			}
			
			rqueue.GetQueue(player_index).pop_back();
			return select_randomness_common(pthis, game_slot, random_input_data, player_index, random_flag, cpu_random_flag, unused, char_id);
		}
	}
	
	return ret;
}

uint32_t select_randomness_stage_common(void *obj, void *random_input_data, bool random_flag, bool cpu_random_flag)
{
	uint32_t stage = select_randomness_stage(obj, random_input_data, random_flag, cpu_random_flag);
	
	if (rqueue_stages.mode < 0)
		return stage;	
	
	Layer2StageSlot *slot = nullptr;
	Layer2StageSlot rslot(0xDEADC0DE);
	
	if (stage_slot)
	{
		stage_slot->mod_index = -1;
		stage_slot = nullptr;
	}
	
	bool use_randomization = true;
		
	auto it = stage_slots.find(stage);
	if (it != stage_slots.end())
	{
		slot = it->second;
	}
	
	if (slot)
	{
		rslot = *slot;
	}
	
	if (!slot || slot->mods.size() == 0 || !enable_random_stage_mods)
	{
		use_randomization = false;
	}
	
	rslot.mod_index = -1;
		
	if (use_randomization)
	{
		int rnd = (int)Utils::RandomInt(0, slot->mods.size()+1); // >= 0, && <= slot->mods.size
	
		if (rnd >= (int)slot->mods.size())
		{
			rnd = -1; // Vanilla
		}
		
		rslot.mod_index = rnd;
	}
	
	if (slot)
	{
		stage = resolve_work_stage(rslot.GetWorkStage());
	}	
	
	if (rqueue_stages.mode == MODE_VS || rqueue_stages.mode == MODE_FREE_TRAINING)
	{
		rqueue_stages.GetQueue().push_back(rslot);
		
		// This has been moved to be done in load_stage
		/*if (rqueue_stages.QueueSize() == 1)
		{
			process_randomness_stage(&rslot);
		}*/
	}
	else if (rqueue_stages.mode == MODE_ARCADE)
	{
		rqueue_stages.GetQueue().push_back(rslot);
		
		// This has been moved to be done in load_stage
		/*if (rqueue_stages.QueueSize() == 1)
		{
			process_randomness_stage(&rslot);
		}*/
	}
		
	return stage;
}

// Patch section referenced by the xml (needs to be in C linking)

extern "C"
{
	
PUBLIC void SetupMRCWI(MRCWIType orig)
{
	model_request_character_with_id = orig;
}

PUBLIC void SetupMCC(MCCType orig)
{
	model_character_completed = orig;
}

PUBLIC void SetupMGDG(MGDGType orig)
{
	model_get_default_glasses = orig;
}

PUBLIC void SetupVGH(VGHType orig)
{
	var_get_hash = orig;
}

PUBLIC void SetupVGB(VGBType orig)
{
	var_get_boolean = orig;
}

PUBLIC void SetupLPA(LPAType orig)
{
	layout_play_anime = orig;
}

PUBLIC void OnMOLocated(void *addr)
{
	pmodel_object = (void **)GetAddrFromRel(addr);
}

PUBLIC void OnCOLocated(void *addr)
{
	pcloset_object = (void **)GetAddrFromRel(addr);
}

/*PUBLIC void OnOOLocated(void *addr)
{
	poption_object = (void **)GetAddrFromRel(addr);
}*/
PUBLIC void OnLOLocated(void *addr)
{
	playout_object = (void **)GetAddrFromRel(addr);
}

PUBLIC void SetupICM(ICMType orig)
{
	is_cached_model = orig;
}

PUBLIC void SetupICF(ICMType orig)
{
	is_cached_face = orig;
}

PUBLIC void SetupBI(BFType orig)
{
	battle_initialize = orig;
}

PUBLIC void SetupBF(BIType orig)
{
	battle_finalize = orig;
}

PUBLIC void SetupBNM(BNMType orig)
{
	battle_next_match = orig;
}

PUBLIC void SetupBRM(BRMType orig)
{
	battle_rematch = orig;
}

PUBLIC void SetupSR(SRType orig)
{
	select_randomness = orig;
}

PUBLIC void SetupSRS(SRSType orig)
{
	select_randomness_stage = orig;
}

PUBLIC void OnCISLLocated(CISLType orig)
{
	mode_closet_initialize_select_list = orig;
}

PUBLIC void OnCRDSLLocated(CRDSLType orig)
{
	mode_closet_reset_detail_select_list = orig;
}

PUBLIC void OnCGDHLIH_NNN_Located(CGDHLIHType orig)
{
	mode_closet_get_detail_hair_list_item_hash = orig;
}

PUBLIC void OnCGDHLS_NNN_Located(CGDHLSType orig)
{
	mode_closet_get_detail_hair_list_size = orig;
}

PUBLIC void OnOGCRFCLocated(OGCRFCType orig)
{
	mode_option_get_character_random_filter_count = orig;
}

PUBLIC void OnCGDFMLocated(CGDFMType orig)
{
	mode_closet_get_different_face_model = orig;
}

PUBLIC void OnLSPTULocated(LSPTUType orig)
{
	layout_set_pane_text_utf8 = orig;
}

PUBLIC void SetupLSPTII(LSPTIIType orig)
{
	layout_set_pane_texture_ii = orig;
}

PUBLIC void SetupLS(LSType orig)
{
	load_stage = orig;
}

PUBLIC void SetupSSP(SSPType orig)
{
	set_stage_params = orig;
}

PUBLIC void SetupGCSBGM(GCSBGMType orig)
{
	get_current_stage_bgm = orig;
}

PUBLIC void SetupLSD(LSDType orig)
{
	load_slot_data = orig;
}

PUBLIC void SetupCLRI(CLRIType orig)
{
	CharacterLoadRequestImpl_ctor = orig;
}

PUBLIC void SetupOCB(EventFuncType orig)
{
	OnCharacterBreak = orig;
}

PUBLIC void SetupOCT(EventFuncType orig)
{
	OnCharacterTransform = orig;
}

PUBLIC void OnPatchCK(uint8_t *addr)
{
	// Patch dinput (keyboard)
	PatchUtils::Write16(addr, 0xE890); 
	PatchUtils::HookCall(addr+1, nullptr, (void *)DirectInputDevice8_GetDeviceState_Patched);
	
	// Patch xinput (pad)
	uintptr_t *iat = (uintptr_t *)PatchUtils::FindImport("XINPUT1_3.dll", (const char *)2, nullptr, true);
	if (!iat)
	{
		FTPRINTF(false, "Error: cannot find XinputGetState.\n");
		return;
	}	
	
	XInputGetState_Orig = (XIGSType)*iat;		
	PatchUtils::Write64(iat, (uint64_t)XinputGetState_Patched);
}

// color2 was added in 1.19 (chinese dress update, and possibly the update where Tamaki was implemented internally)
PUBLIC int model_request_character_with_id_patched(void *pthis, uint32_t player_index, uint32_t chara_hash, uint32_t costume_hash, uint32_t face_hash, uint32_t hair_hash, uint32_t color, int hair_color)
{
	//DPRINTF("------ model::request_character_with_id (called from %p)\nplayer_index: %x\nchara_hash: 0x%08x\n", BRA(0), player_index, chara_hash);
	if (hair_color != -1)
	{
		DPRINTF("Notice: hair_color != -1 (model::request_character_with_id %d 0x%08x, 0x%08x, 0x%08x, 0x%08x %d %d, RA: %p)\n", player_index, chara_hash, costume_hash, face_hash, hair_hash, color, hair_color, BRA(0));
	}
	
	bool force_disable_cache = ((player_index & 0x80000000) != 0);
	player_index &= 0x7FFFFFFF;
	
	if (chara_hash == 0 && costume_hash == 0xc6de52db && face_hash == 0xbc28f906 && player_index == 1) // This is a work around for Nico face edits not working
	{
		return 0;
	}
	
	/*auto it_cos = hash_to_costume.find(costume_hash);
	if (it_cos != hash_to_costume.end())
	{
		DPRINTF("costume: %s (0x%08x)\n", it_cos->second.c_str(), costume_hash);
	}
	else
	{
		DPRINTF("costume_hash: 0x%08x\n", costume_hash);
	}
	
	auto it_face = hash_to_face.find(face_hash);
	if (it_face != hash_to_face.end())
	{
		DPRINTF("face: %s (0x%08x)\n", it_face->second.c_str(), face_hash);
	}
	else
	{	
		DPRINTF("face_hash: 0x%08x\n", face_hash);
	}
	
	auto it_hair = hash_to_hair.find(hair_hash);
	if (it_hair != hash_to_hair.end())
	{
		DPRINTF("hair: %s (0x%08x)\n", it_hair->second.c_str(), hair_hash);
	}
	else
	{
		DPRINTF("hair_hash: 0x%08x\n", hair_hash);
	}
	
	DPRINTF("color = %d\n------------\n", color); */
	
	//if (css_index >= 0 || dss_index >= 0)	
	if (true)
	{	
		auto it = slots.find(costume_hash | ((uint64_t)player_index << 32));		
		
		if (it != slots.end() && player_index < NUM_PLAYERS)
		{
			Layer2Slot *slot = it->second;			
			
			//if (player_slots[player_index] != slot)
			if (true) // Fixes color shit
			{
				Layer2Slot *prev_slot = player_slots[player_index];
				Layer2Slot prev_slot_copy(0, 0, 0);
				uint32_t prev_hair = 0;
				
				disable_cache = false;
				
				if (prev_slot)
				{
					prev_slot_copy = *prev_slot;
				}
				
				if (prev_slot && prev_slot != slot)
				{
					if (prev_slot->chara_owner == slot->chara_owner)
					{
						prev_hair = prev_slot->current_hair;
					}
				}
				else if (!prev_slot)
				{
					slot->mod_index = -1;
					//slot->SetCurrentHairModIndex(-1);
					disable_cache = true;
				}
				
				player_slots[player_index] = slot;	
				slot->chara_hash = chara_hash;
				slot->costume = costume_hash;
				slot->current_face = face_hash;
				slot->current_hair = hair_hash;
				slot->current_color = color;
				slot->current_hair_color = hair_color;
				
				if (prev_hair != 0 && prev_hair != slot->current_hair)
				{
					//slot->SetCurrentHairModIndex(-1);
				}
				
				if (force_disable_cache)
					disable_cache = true;
				
				else
				{				
					if (prev_slot && prev_slot_copy != *slot)
						disable_cache = true;					
				}				
				
				//DPRINTF("-------------- %d %d %d\n", slot->player_index, slot->mod_index, slot->hair_mod_index);	

				// Resolve default hair (if any)
				// Currently disabled, too many problems
				/*
				if (dss_index < 0) // Only do it if detail screen is not up
				{
					DPRINTF("Resolving default hair.\n");
					Layer2Mod *mod = slot->GetCurrentMod();
					if (mod && mod->default_hair != 0)
					{
						slot->current_hair = mod->default_hair;
						bool ret = slot->SetCurrentHairMod(mod->default_hair_mod);
						DPRINTF("Hair resolved: %d\n", ret);
					}
				}
				else
				{
					DPRINTF("Skipping default hair because detail screen up.\n");
				}*/
				
				
				costume_hash = slot->GetWorkCostume();
				face_hash = slot->GetWorkFace();
				hair_hash = slot->GetWorkHair();
				
				
				rdbemu_set_layer2_mode(player_index);
				
				rdbemu_set_layer2(slot->GetCurrentMod(), player_index);				
				rdbemu_set_layer2_hair(slot->GetCurrentHairMod(), player_index); 
				
				show_slot_info(slot);
			}
		}		
	}
	
	//DPRINTF("disable_cache = %d\n", disable_cache); 
	MutexLocker lock(&reqm_mutex);
	int ret = model_request_character_with_id(pthis, player_index, chara_hash, costume_hash, face_hash, hair_hash, color, hair_color);	
	disable_cache = false;
	
	return ret;
}

PUBLIC uint32_t model_get_default_glasses_patched(void *pthis, uint32_t costume_hash)
{
	int player_index = (css_index >= 0) ? css_index : dss_index;
	
	if (player_index >= 0 && player_index < NUM_PLAYERS)
	{
		Layer2Slot *slot = player_slots[player_index];
		
		if (slot && slot->costume == costume_hash)
		{
			Layer2Mod *mod = slot->GetCurrentMod();
			if (mod && mod->has_default_glasses)
			{
				DPRINTF("Setting default glasses to 0x%08x\n", mod->default_glasses);
				return mod->default_glasses;	
			}
		}
	}
	
	return model_get_default_glasses(pthis, costume_hash);
}

PUBLIC uint32_t get_battle_costume_patched(void *pthis, uint32_t row, uint32_t col, uint32_t player_index)
{
	uint32_t hash = var_get_hash(pthis, row, col, player_index);
	
	//DPRINTF("battle_costume %d, 0x%08x\n", player_index, hash);
	got_battle_costume = true;
	
	if (listener_installed)
		uninstall_listener();
	
	if (player_index < NUM_PLAYERS)
	{
		Layer2Slot *slot = player_slots[player_index];
		
		if (slot)
		{		
			if (hash == 0 || slot->costume != hash)
			{
				slot->mod_index = -1;	
				slot->SetCurrentHairModIndex(-1);
				player_slots[player_index] = nullptr;
			}
			else 
			{
				// This moved to get_random_characer_flag_patched
				/*if (slot->IsRandom())
					return 0;*/
				
				if (slot->IsTransform())
				{
					if (player_index == 0)
						transform_p1 = true;
					else
						transform_p2 = true;
				}
				else
				{
					if (player_index == 0)
						transform_p1 = false;
					else
						transform_p2 = false;
				}
				
				return slot->GetWorkCostume();
			}
		}
	}
	
	return hash;
}

PUBLIC uint32_t get_battle_face_patched(void *pthis, uint32_t row, uint32_t col, uint32_t player_index)
{
	uint32_t hash = var_get_hash(pthis, row, col, player_index);
	
	if (player_index < NUM_PLAYERS)
	{
		Layer2Slot *slot = player_slots[player_index];
		
		if (slot)
		{		
			if (hash == slot->current_face) 
			{
				return slot->GetWorkFace();
			}
		}
	}
	
	return hash;
}

PUBLIC uint32_t get_battle_hair_patched(void *pthis, uint32_t row, uint32_t col, uint32_t player_index)
{
	uint32_t hash = var_get_hash(pthis, row, col, player_index);
	
	if (player_index < NUM_PLAYERS)
	{
		Layer2Slot *slot = player_slots[player_index];
		
		if (slot)
		{		
			if (hash == slot->current_hair) 
			{
				hash = slot->GetWorkHair();
			}
		}
	}
	
	if (player_index == 1)
		battle_select_end();
	
	return hash;
}

PUBLIC uint32_t get_battle_stage_patched(void *pthis, uint32_t row, uint32_t col, uint32_t extra)
{
	uint32_t hash = var_get_hash(pthis, row, col, extra);
	//DPRINTF("stage hash = 0x%08x\n", hash);
	
	if (stage_slot)
	{
		if (hash == 0 || hash != stage_slot->stage)
		{
			stage_slot = nullptr;
		}
		else
		{
			hash = resolve_work_stage(stage_slot->GetWorkStage());
		}
	}
	
	if (stage_slot)
	{
		rdbemu_set_layer2_stage(stage_slot->GetCurrentMod());
	}
	else
	{
		rdbemu_set_layer2_stage(nullptr);
	}
	
	return hash;
}

PUBLIC bool get_random_character_flag_patched(void *pthis, uint32_t row, uint32_t col, uint32_t player_index)
{
	Layer2Slot *slot = player_slots[player_index];
	if (slot && slot->IsRandom())
		return true;
	
	return var_get_boolean(pthis, row, col, player_index);
}

PUBLIC uintptr_t layout_play_anime_patched(void *pthis, uint32_t hash, const char *str, uint32_t unk_r9d)
{
	uintptr_t ret = layout_play_anime(pthis, hash, str, unk_r9d); // It is really a void func, but let's preserve rax, just in case...	
	
	//DPRINTF("LPA 0x%x %s\n", hash, str);
	
	if (!str)
		return ret;
	
	if (hash == 0xb384f573 && *(uint32_t *)str == 0x5F736F63) // "cos_"
	{
		int css_index_prev = css_index;
		
		if (memcmp(str+4, "in_p", 4) == 0)
		{		
			int16_t n = *(int16_t *)(str+8); // int16_t to include the null terminating
			
			if (n == '1')
			{
				css_index = 0;
			}
			else if (n == '2')
			{
				css_index = 1;
			}
		}
		else if (memcmp(str+4, "out_p", 5) == 0)
		{
			int16_t n = *(int16_t *)(str+9); // int16_t to include the null terminating
			
			if (n == '1')
			{
				if (css_index == 0)
					css_index = -1;
			}
			else if (n == '2')
			{
				if (css_index == 1)
					css_index = -1;
			}
		}
		
		if (css_index != css_index_prev)
		{
			//DPRINTF("css_index = %d\n", css_index); 
			
			if (css_index >= 0 && !listener_installed)
				install_listener();
		}
	}
	else if (hash == 0xdd6728d2 && memcmp(str, "detail_", 7) == 0)
	{
		int dss_index_prev = dss_index;
		
		if (strcmp(str+7, "pos_1p") == 0)
		{
			dss_index = 0;
		}
		else if (strcmp(str+7, "pos_2p") == 0)
		{
			dss_index = 1;
		}
		else if (strcmp(str+7, "out") == 0)
		{
			dss_index = -1;
		}
		
		if (dss_index != dss_index_prev)
		{
			//DPRINTF("dss_index = %d\n", dss_index); 
		}
	}
	else if (hash == 0xd0805c2d && strcmp(str, "stage_sele_base_out") == 0)
	{
		bool in_sss_prev = in_sss;		
		in_sss = false;
		
		if (in_sss != in_sss_prev)
		{
			//DPRINTF("in_sss = %d\n", in_sss);
		}
	}
	
	return ret;
}

PUBLIC bool is_cached_model_patched(void *rcx, uintptr_t rdx, uintptr_t r8, uintptr_t r9)
{
	if (disable_cache)
		return false;
	
	return is_cached_model(rcx, rdx, r8, r9);
}

PUBLIC bool is_cached_face_patched(void *rcx, uintptr_t rdx, uintptr_t r8, uintptr_t r9)
{
	if (disable_cache)
		return false;
	
	return is_cached_face(rcx, rdx, r8, r9);
}

PUBLIC void battle_initialize_patched(void *pthis)
{
	//DPRINTF("battle_initialize\n");
	
	for (int i = 0; i < 2; i++)
		saved_random_slots[i] = Layer2Slot(0, 0, 0);
	
	got_battle_costume = false;
	rqueue.Reset();
	rqueue_stages.Reset();
	
	player1_random = player2_random = false;
	next_player_index = 0;
	fight_num = 0;		
	transform_p1 = transform_p2 = false;
	
	in_battle = true;
	uninstall_listener();
	toggle_battle_hud_reset();
	
	battle_initialize(pthis);	
}

PUBLIC void battle_finalize_patched(void *pthis)
{
	//DPRINTF("battle_finalize\n");
	in_battle = false;
	
	for (int i = 0; i < 2; i++)
		saved_random_slots[i] = Layer2Slot(0, 0, 0);
	
	rqueue.Reset();
	rqueue_stages.Reset();
	
	/*for (int i = 0; i < NUM_PLAYERS; i++)
	{	
		Layer2Slot *slot = player_slots[i];
		if (slot)
		{
			slot->mod_index = slot->hair_mod_index = -1;
		}
		
		player_slots[i] = nullptr;
	}*/
	
	for (auto &it : slots)
	{
		if (it.second != player_slots[0] && it.second != player_slots[1])
		{
			it.second->mod_index = -1;
			it.second->SetCurrentHairModIndex(-1);
		}
	}
	
	player1_random = player2_random = false;
	next_player_index = 0;
	fight_num = 0;
	transform_p1 = transform_p2 = false;
	
	battle_finalize(pthis);
}

PUBLIC void battle_next_match_patched(void *pthis)
{
	//DPRINTF("battle_next_match\n");
	
	if (old_random_mode)
	{
		if (rqueue.mode == MODE_ARCADE || rqueue.mode == MODE_SURVIVAL)
		{
			rqueue.Advance(1);
			
			if (rqueue.mode == MODE_SURVIVAL && rqueue.next_p2 == 1)
			{
				rqueue.Advance(1); 
			}
			
			Layer2Slot *rslot2 = rqueue.GetNextSlot(1);
			
			if (rslot2)
				process_randomness(rslot2, 1); // Remainder: for survival we are actually loading now not the char for this match, but for next.
		}
	}
	
	// Old code, we do things now in load_stage_patched and set_stage_params_patched 
	/*if (rqueue_stages.mode == MODE_ARCADE)
	{
		rqueue_stages.Advance();
		
		Layer2StageSlot *rslot = rqueue_stages.GetNextSlot();
		
		if (rslot)
			process_randomness_stage(rslot);
	}*/
	
	battle_next_match(pthis);
}

PUBLIC void battle_rematch_patched(void *pthis)
{
	//DPRINTF("battle_rematch\n");
	if (old_random_mode)
	{	
		if (rqueue.mode == MODE_VS)
		{
			for (int i = 0; i < NUM_PLAYERS; i++)
			{
				rqueue.Advance(i);
				Layer2Slot *rslot = rqueue.GetNextSlot(i);
				
				if (rslot)
					process_randomness(rslot, i);
			}
		}
	}
	
	// Old code, we do things now in load_stage_patched and set_stage_params_patched 
	/*if (rqueue_stages.mode == MODE_VS)
	{
		rqueue_stages.Advance();
		Layer2StageSlot *rslot = rqueue_stages.GetNextSlot();
		
		if (rslot)
			process_randomness_stage(rslot);
	}*/
	
	battle_rematch(pthis);
}

PUBLIC void *CharacterLoadRequestImpl_ctor_Patched(void *pthis, void *unk2, uint8_t char_code, GameCharSlot2 *slot, uint8_t unk5)
{
	//DPRINTF("Load request impl ctor 0x%08x, pi = %d, mode = %d\n", slot->costume_hash, next_player_index, rqueue.mode);
	
	if (rqueue.mode >= 0)
	{
		if (next_player_index >= 0 && !old_random_mode)
		{
			bool is_random = (next_player_index == 0) ? player1_random : player2_random;			
			
			if (is_random)
			{
				if (next_player_index == 0)
					transform_p1 = false;
				else
					transform_p2 = false;
				
				Layer2Slot *rslot = rqueue.GetNextSlot(next_player_index);
				rqueue.Advance(next_player_index);
				
				if (rslot)
				{
					// These two conditions can happen without error, when the game detects an invalid configuration in the random list, such as that the combination of player+costume is identical in player2 than player1
					// We fix this, by recursing, which will advance our queue aswell.
					// In case of weirdness, there is no risk of infinite loop, because eventually rslot would be null
					uint8_t r_id = character_hash_to_id(rslot->chara_owner);
					
					if (rslot->GetWorkCostume() != slot->costume_hash || r_id != char_code)
					{						
						DPRINTF("Warning: there may be desync in random queue, costume != work costume or char != owner (0x%08x != 0x%08x or %d != %d) (mode=%d) (under certain conditions, this warning is normal)\n", slot->costume_hash, rslot->GetWorkCostume(), char_code, r_id, rqueue.mode);						
						return CharacterLoadRequestImpl_ctor_Patched(pthis, unk2, char_code, slot, unk5);
					}
					
					if (rslot->GetWorkHair() != slot->hair_hash)
					{
						DPRINTF("Warning: there may be desync in random queue, hair != work hair (0x%08x != 0x%08x) (mode=%d)\n", slot->hair_hash, rslot->GetWorkHair(), rqueue.mode);
					}
					
					process_randomness(rslot, next_player_index);	

					if (rslot->IsTransform())
					{
						if (next_player_index == 0) 
							transform_p1 = true;
						else 
							transform_p2 = true;
					}					
				}
				else
				{
					DPRINTF("Warning: rslot null when not expected (costume = 0x%08x, mode = %d).\n", slot->costume_hash, rqueue.mode);							
				}
			}
			
			if (rqueue.mode == MODE_VS || rqueue.mode == MODE_FREE_TRAINING)
			{
				if (fight_num == 0)
				{
					if (next_player_index == 0)
					{
						next_player_index = 1;
					}
					else
					{
						fight_num++;
						
						if (player1_random)
							next_player_index = 0;
						else if (player2_random)
							next_player_index = 1; // No change
						else
							next_player_index = -1; // There is no random slot. 
					}
				}
				else
				{
					if (next_player_index == 0)
					{
						if (player2_random)
						{
							next_player_index = 1;
						}
						else
						{
							fight_num++;
							next_player_index = 0; // No change
						}
					}
					else
					{
						fight_num++;
						
						if (player1_random)
						{
							next_player_index = 0;
						}
						else
						{
							next_player_index = 1; // No change
						}
					}
				}
			}
			else if (rqueue.mode == MODE_ARCADE || rqueue.mode == MODE_SURVIVAL)
			{
				if (next_player_index == 0)
					next_player_index = 1;
				else
					fight_num++;
			}	
			else
			{
				DPRINTF("ERROR: Unknown rqueue.mode %d\n", rqueue.mode);
			}
		}
	}
	else
	{
		if (!got_battle_costume && next_player_index >= 0)
		{
			Layer2Slot *pslot = player_slots[next_player_index];
			
			if (pslot)
			{
				if (pslot->costume == slot->costume_hash)
				{
					DPRINTF("Warning: battle costume couldn't be obtained, resolving layer2 now (this warning is expected if in online).\n");
					
					rdbemu_set_layer2_mode(-1);
					
					Layer2Mod *mod = pslot->GetCurrentMod();
					Layer2Mod *hair_mod = pslot->GetCurrentHairMod();
					
					rdbemu_set_layer2(mod, next_player_index);
					rdbemu_set_layer2_hair(hair_mod, next_player_index);
				}
			}
			
			if (next_player_index == 0)
				next_player_index = 1;
			else
				next_player_index = -1;
		}
	}
	
	return CharacterLoadRequestImpl_ctor(pthis, unk2, char_code, slot, unk5);
}

PUBLIC void *select_randomness_arcade_ta(void *pthis, GameCharSlot *game_slot, void *random_input_data, uint32_t player_index, bool random_flag, bool cpu_random_flag, uintptr_t unused, uint8_t char_id)
{
	//DPRINTF("Select randomness Arcade/TA: %d\n", player_index);	
	rqueue.mode = MODE_ARCADE;
	
	return select_randomness_common(pthis, game_slot, random_input_data, player_index, random_flag, cpu_random_flag, unused, char_id);
}

PUBLIC void *select_randomness_survival(void *pthis, GameCharSlot *game_slot, void *random_input_data, uint32_t player_index, bool random_flag, bool cpu_random_flag, uintptr_t unused, uint8_t char_id)
{
	//DPRINTF("Select randomness survival: %d\n", player_index);
	rqueue.mode = MODE_SURVIVAL;
	
	if (player_index == 0)
	{
		// Handle survival stage now, before the game loads it
		if (survival_stage_slot != 0)
		{
			Layer2StageSlot *slot = nullptr;
			
			auto it = stage_slots.find(survival_stage_slot);
			if (it != stage_slots.end())
			{
				slot = it->second;				
			}
			
			if (slot)
			{
				stage_slot = slot;
				
				slot->SetCurrentMod(survival_stage_mod);
				rdbemu_set_layer2_stage(survival_stage_mod);				
			}
		}
	}
	
	return select_randomness_common(pthis, game_slot, random_input_data, player_index, random_flag, cpu_random_flag, unused, char_id);
}

PUBLIC void *select_randomness_versus(void *pthis, GameCharSlot *game_slot, void *random_input_data, uint32_t player_index, bool random_flag, bool cpu_random_flag, uintptr_t unused, uint8_t char_id)
{
	//DPRINTF("Select randomness VS %d\n", player_index);
	rqueue.mode = MODE_VS;
	
	return select_randomness_common(pthis, game_slot, random_input_data, player_index, random_flag, cpu_random_flag, unused, char_id);
}

PUBLIC void *select_randomness_ft(void *pthis, GameCharSlot *game_slot, void *random_input_data, uint32_t player_index, bool random_flag, bool cpu_random_flag, uintptr_t unused, uint8_t char_id)
{
	//DPRINTF("Select randomness free training %d\n", player_index);
	rqueue.mode = MODE_FREE_TRAINING;
	
	return select_randomness_common(pthis, game_slot, random_input_data, player_index, random_flag, cpu_random_flag, unused, char_id);
}

PUBLIC uint32_t select_randomness_stage_arcade_ta(void *pthis, void *random_input_data, bool random_flag, bool cpu_random_flag)
{
	//DPRINTF("Select randomness Arcade/TA: %d\n", player_index);	
	rqueue_stages.mode = MODE_ARCADE;	
	return select_randomness_stage_common(pthis, random_input_data, random_flag, cpu_random_flag);
}

PUBLIC uint32_t select_randomness_stage_versus(void *pthis, void *random_input_data, bool random_flag, bool cpu_random_flag)
{
	//DPRINTF("Select randomness VS %d\n", player_index);
	rqueue_stages.mode = MODE_VS;	
	return select_randomness_stage_common(pthis, random_input_data, random_flag, cpu_random_flag);
}

PUBLIC uint32_t select_randomness_stage_ft(void *pthis, void *random_input_data, bool random_flag, bool cpu_random_flag)
{
	//DPRINTF("Select randomness VS %d\n", player_index);
	rqueue_stages.mode = MODE_FREE_TRAINING;	
	return select_randomness_stage_common(pthis, random_input_data, random_flag, cpu_random_flag);
}

PUBLIC void fix_random_glasses(void *rcx, void *rdx, uint16_t battle_index, bool allow_random_glasses)
{
	allow_random_glasses = true;
	load_slot_data(rcx, rdx, battle_index, allow_random_glasses);
}

PUBLIC int model_character_completed_patched(void *pthis, uint32_t player_index, uint32_t costume_hash, uint32_t face_hash, uint32_t hair_hash)
{
	int ret = model_character_completed(pthis, player_index, costume_hash, face_hash, hair_hash);
	if (ret)
	{
		disable_cache = false;
	}
	
	/*DPRINTF("------ model::character_completed (called from %p)\nplayer_index: %d\n", BRA(0), player_index);
	
	auto it = hash_to_costume.find(costume_hash);
	if (it != hash_to_costume.end())
	{
		DPRINTF("costume: %s\n", it->second.c_str());
	}
	else
	{
		DPRINTF("costume_hash: 0x%08x\n", costume_hash);
	}
	
	DPRINTF("face_hash: 0x%08x\nhair_hash: 0x%08x\nRet = %d\n----------------------\n", face_hash, hair_hash, ret);*/
	return ret;
}

PUBLIC void layout_set_pane_texture_ii_patched(void *obj, uint32_t hash, uint32_t t1, uint32_t t2, const char *fmt, uint32_t n1, uint32_t n2)
{
	if (hash == 0xd0805c2d && strcmp(fmt, "stage_%02d_%d") == 0)
	{
		bool in_iss_prev = in_sss;		
		in_sss = true;
		
		if (in_iss_prev != in_sss)
		{
			//DPRINTF("in_sss %d\n", in_sss);
			if (!listener_installed)
				install_listener();
			
			rdbemu_set_layer2_stage(nullptr);
		}
		//DPRINTF("Stage %d %d %d %d\n", t1, t2, n1, n2);		

		uint32_t stage = get_stage_by_indexes(n1, n2);
		if (stage != 0)
		{
			/*auto it = hash_to_stage.find(stage);
			if (it != hash_to_stage.end())
			{
				DPRINTF("Stage %s\n", it->second.c_str());
			}
			else
			{
				DPRINTF("Stage 0x%08x\n", stage);
			}*/
		}
		
		auto it = stage_slots.find(stage);
		if (it == stage_slots.end())
		{
			stage_slot = nullptr;
			layout_set_pane_texture_ii(obj, hash, t1, t2, fmt, n1, n2);
			return;
		}
		
		stage_slot = it->second;		
		show_stage_slot_info(stage_slot);		
	}
	
	layout_set_pane_texture_ii(obj, hash, t1, t2, fmt, n1, n2);
}

PUBLIC void *load_stage_patched(void *obj, void *unused, uint32_t stage_hash, uint32_t param, void *unk)
{
	/*auto it = hash_to_stage.find(stage_hash);
	if (it != hash_to_stage.end())
	{
		DPRINTF("Load stage %s 0x%08x\n", it->second.c_str(), param);
	}
	else
	{
		DPRINTF("Load stage 0x%08x 0x%08x\n", stage_hash, param);
	}*/	
	
	int type = 0;
	
	if (stage_hash == 0xf42ace19 || stage_hash == 0x009b8dfb) // S1102BAM, S1199BAM
		type = 1;
	else if (stage_hash == 0x17a511b2) // S0401CLS
		type = 2;
	else if (stage_hash == 0x3c91a512) // S0901WAY
		type = 3;
	else if (stage_hash == 0x4e2a96cd) // S0201BST
		type = 4;	
	
	Layer2StageSlot *slot = nullptr;
	
	if (rqueue_stages.mode >= 0)
	{
		slot = rqueue_stages.GetNextSlot();
		
		if (slot)
		{
			bool advance = true;
			process_randomness_stage(slot);
			
			auto it = final_stages.find(resolve_work_stage(slot->GetWorkStage()));
			if (it != final_stages.end())
			{
				advance = (stage_hash == it->second);
			}
			
			auto it2 = stage_slots.find(slot->stage);
			if (it2 != stage_slots.end())
			{
				*(it2->second) = *slot;
			}

			if (advance)
			{				
				rqueue_stages.Advance();
			}
		}
	}
	else
	{
		slot = stage_slot;
	}

	if (slot && param == 0 && type != 0 && is_special_stage(slot->GetWorkStage()))
	{
		bool is_bossl = is_bossl_stage(slot->GetWorkStage());
		
		if (type == 1)
		{
			param = (is_bossl) ? 0xbc80ce64: 0x87171e40;	
		}
		else if (type == 2)
		{
			param = 0xe1afea9c;
		}
		else if (type == 3)
		{
			param = 0x41bfa289;
		}
		else if (type == 4)
		{
			param = 0x32ec82f2;
		}
	}	
	
	return load_stage(obj, unused, stage_hash, param, unk);
}

static Layer2StageSlot *get_ss(uint32_t current_stage)
{
	Layer2StageSlot *slot = nullptr;
	
	if (rqueue_stages.mode >= 0)
	{
		uint32_t s = current_stage;
		if (s == 0x17b32933) s = 0x17a511b2; // S0411CLS -> S0401CLS
		
		auto it = stage_slots.find(s);
		if (it != stage_slots.end())
			slot = it->second;
	}
	else if (rqueue.mode == MODE_SURVIVAL && survival_stage_slot == 0)
	{
		slot = nullptr;
	}
	else
	{
		slot = stage_slot;
		
		if (slot && resolve_work_stage(slot->GetWorkStage()) != current_stage)
			slot = nullptr;
	}
	
	return slot;
}

PUBLIC void set_stage_params_patched(void *pthis, void *rdx, StageDetail *details, uint32_t r9d)
{
	uint32_t current_stage = get_current_stage();
	
	/*auto it = hash_to_stage.find(current_stage);
	if (it != hash_to_stage.end())
	{
		DPRINTF("set_stage_params %s 0x%08x\n", it->second.c_str(), details->param2);
	}
	else
	{
		DPRINTF("set_stage_params 0x%08x 0x%08x\n", current_stage, details->param2);
	}*/
	
	Layer2StageSlot *slot = get_ss(current_stage);
	
	//DPRINTF("slot %p\n", slot);
	
	if (slot)
	{
		Layer2Mod *mod = slot->GetCurrentMod();
		if (mod && mod->disable_npc)
		{
			uint32_t param2 = 0;
			uint32_t stage = resolve_work_stage(slot->GetWorkStage());
			
			switch (stage)
			{
				case 0x17a511b2: // S0401CLS
					param2 = 0x97d0be76;
				break;
				
				case 0x4e2a96cd: // S0201BST
					param2 = 0xf16f9ea7;
				break;
				
				case 0x0d511087: case 0x0f05e926: // S0301CRM, S0302CRM
					param2 = 0xd0c52e4c;
				break;
				
				case 0xb8123b65: // S1301GYM
					param2 = 0xf4172d90;
				break;
			}
			
			//DPRINTF("param2 = 0x%08x\n", param2);
			
			if (param2 != 0)
			{
				details->param2 = param2;
				details->no_mob_flag = true;
			}
		}
	}
	
	set_stage_params(pthis, rdx, details, r9d);
}

PUBLIC uint32_t get_current_stage_bgm_patched()
{
	uint32_t bgm = misc_process_bgm();	
	if (bgm != 0)
		return bgm;
	
	bgm = get_current_stage_bgm();
	uint32_t current_stage = get_current_stage();
	
	/*auto it = hash_to_stage.find(current_stage);
	if (it != hash_to_stage.end())
	{
		DPRINTF("get_current_stage_bgm %s: 0x%08x\n", it->second.c_str(), bgm);
	}
	else
	{
		DPRINTF("get_current_stage_bgm 0x%08x 0x%08x\n", current_stage, bgm);
	}*/
	
	Layer2StageSlot *slot = get_ss(current_stage);
	if (slot)
	{
		Layer2Mod *mod = slot->GetCurrentMod();
		if (mod && mod->bgm != 0)
		{
			return mod->bgm;
		}
	}
	
	return bgm;
}

PUBLIC bool IsSurvivalPatchNeeded()
{
	std::string stage;	
	
	bool ret = true;
	
	if (ini.GetStringValue("Misc", "survival_stage", stage) && stage.length() > 0)
	{
		uint32_t hash = hash_func(Utils::ToUpperCase(stage));
		if (!stage_exists(hash))
		{
			// Then assume it is a mod
			if (!Utils::DirExists(Utils::MakePathString(myself_path + "REDELBE/Layer2", stage)))
			{
				DPRINTF("Warning: Misc:survival_stage_mod: mod \"%s\" not found.\n", stage.c_str());
				ret = false;
			}
			else
			{
				survival_stage_mod_str = stage;
			}			
		}
		else if (hash == 0x17a511b2) // Colosseum)
		{
			ret = false;
		}
		else
		{
			survival_stage = hash;
		}
	}
	else
	{
		ret = false;
	}	
	
	return ret;
}

PUBLIC void OnSurvivalStageLocated(uint32_t *survival_stage)
{
	p_survival_stage = survival_stage;
}

PUBLIC bool OnCharacterBreakPatched(void *pthis)
{
	uint8_t player_index = PatchUtils::Read8(pthis, 0x3560); 
	bool transform = (player_index == 0) ? transform_p1 : transform_p2;
	
	if (transform)
	{
		uintptr_t ptr = PatchUtils::Read64(pthis, 0x35A8) + 4;
		PatchUtils::WriteData64(pthis, ptr, 0x35A8);	
		
		return true;
	}
	
	return OnCharacterBreak(pthis);
}

PUBLIC bool OnCharacterTransformPatched(void *pthis)
{
	typedef struct
	{
		uint64_t u1, u2;
	} UnkStruct;
	CHECK_STRUCT_SIZE(UnkStruct, 0x10);
	
	uint8_t player_index = PatchUtils::Read8(pthis, 0x3560); 
	bool transform = (player_index == 0) ? transform_p1 : transform_p2;
	
	if (!transform)	// Replace by if not transform
		return OnCharacterTransform(pthis);		
	
	uintptr_t ptr = PatchUtils::Read64(pthis, 0x35A8) + 2;
	PatchUtils::WriteData64(pthis, ptr, 0x35A8);	
		
	void *player = get_player_mao(player_index);
	if (!player)
		return true;
	
	/*uint32_t data = PatchUtils::Read16((void *)ptr);
	PatchUtils::InvokeVirtualRegisterFunction(player, 0x50, data);*/
	
	void *obj = ((uint8_t *)player) + 0x1AD0;
	uint8_t x;
	UnkStruct st;
	
	break_requirement1(&x, (void *)PatchUtils::Read64(obj, 8));
	break_requirement2(&st, x);	
	
	void *dce = *pdamaged_character_event_object;
	uint64_t val = st.u1;
	
	while (dce)
	{
		PatchUtils::InvokeVirtualRegisterFunction(dce, 8, (uintptr_t)&val);
		dce = (void *)PatchUtils::Read64(dce, 8);
	}	
	
	return true;
}

PUBLIC void OnGetPlayerMAOLocated(void *addr)
{
	get_player_mao = (GPMAOType)GetAddrFromRel(addr);
}

PUBLIC void OnBR1Located(void *addr)
{
	break_requirement1 = (BR1Type)GetAddrFromRel(addr);
}

PUBLIC void OnBR2Located(void *addr)
{
	break_requirement2 = (BR2Type)GetAddrFromRel(addr);
}

PUBLIC void OnDCELocated(void *addr)
{
	pdamaged_character_event_object = (void **)GetAddrFromRel(addr);
}

///// Added in 3.1
PUBLIC void SetupDefaults(GRFType orig)
{
	get_related_face = orig;
	
	if (ini.LoadFromFile("REDELBE/defaults.ini"))
	{
		std::vector<std::string> names, values;
		size_t num = ini.GetAllStringsValues("Face", names, values);
		
		for (size_t i = 0; i < num; i++)
		{
			uint32_t costume = hash_func(names[i]);
			uint32_t face = hash_func(values[i]);
			
			auto it = hash_to_costume.find(costume);
			if (it == hash_to_costume.end())			
			{
				FTPRINTF(false, "Error: costume \"%s\" specified in defaults.ini doesn't exist.\n", names[i].c_str());
				return;
			}
			
			auto it2 = hash_to_face.find(face);
			if (it2 == hash_to_face.end())			
			{
				FTPRINTF(false, "Error: face \"%s\" specified in defaults.ini doesn't exist.\n", values[i].c_str());
				return;
			}
			
			defaults_faces[costume] = face;
		}
		
		num = ini.GetAllStringsValues("Hair", names, values);
		
		for (size_t i = 0; i < num; i++)
		{
			uint32_t costume = hash_func(names[i]);
			uint32_t hair = hash_func(values[i]);
			
			auto it = hash_to_costume.find(costume);
			if (it == hash_to_costume.end())			
			{
				FTPRINTF(false, "Warning: costume \"%s\" specified in defaults.ini doesn't exist.\n", names[i].c_str());
				return;
			}
			
			auto it2 = hash_to_hair.find(hair);
			if (it2 == hash_to_hair.end())			
			{
				FTPRINTF(false, "Warning: hair \"%s\" specified in defaults.ini doesn't exist.\n", values[i].c_str());
				return;
			}
			
			defaults_hairs[costume] = hair;
		}
	}
}

PUBLIC void SetupGRH(GRHType orig)
{
	get_related_hair = orig;
}

PUBLIC void SetupLC(LCType orig)
{
	load_costumes = orig;
}

void patch_costumes_list()
{
	//DPRINTF("Begin dump.\n");	
	
	for (uint32_t i = CHAR_ZAC; i < CHAR_NUM; i++)
	{
		void *data1 = (void *)PatchUtils::Read64(0x257C0F0);
		if (!data1)
			return;
		
		CostumeLinkedList *list = (CostumeLinkedList *)PatchUtils::Read64(data1, 0x90); // rbx
		if (!list)
			return;
		
		CostumeLinkedList *list2 = list; // r15
		
		CostumeLinkedList **pnext2 = &list->next2; // r9
		CostumeLinkedList *next2 = list->next2; // rax
		
		uint32_t char_hash = PatchUtils::Read32(data1, i*4);
		//DPRINTF("Char = 0x%08x\n", char_hash);
		
		while (next2->unk_19 == 0)
		{
			if (next2->char_hash < char_hash)
			{
				next2 = next2->next3;
			}
			else 
			{
				if (list2->unk_19 != 0 && char_hash < next2->char_hash)
				{
					list2 = next2;	
				}
				
				list = next2;
				next2 = next2->next;
			}			
		}
		
		next2 = list2;
		
		CostumeLinkedList *list3; // rcx
		
		if (list2->unk_19 != 0)
		{
			list3 = *pnext2;
		}
		else
		{
			list3 = next2->next;
		}	

		while (list3->unk_19 == 0)
		{
			if (char_hash < list3->char_hash)
			{
				list2 = list3;
				list3 = list3->next;
			}
			else
			{
				list3 = list3->next3;
			}
		}
		
		while (list != list2)
		{
			//DPRINTF("%s %s %s %08x\n", get_costume_name(list->costume_hash).c_str(), get_face_name(list->face_hash).c_str(), get_hair_name(list->hair_hash).c_str(), list->unk_30);
			if (true)
			{
				auto it = defaults_hairs.find(list->costume_hash);
				if (it != defaults_hairs.end())
				{
					//DPRINTF("(In list) Changed costume %s hair from %s to %s\n", get_costume_name(list->costume_hash).c_str(), get_hair_name(list->hair_hash).c_str(), get_hair_name(it->second).c_str());
					list->hair_hash = it->second;					
				}
				
				auto it2 = defaults_faces.find(list->costume_hash);
				if (it2 != defaults_faces.end())
				{
					//DPRINTF("(In list) Changed costume %s face from %s to %s\n", get_costume_name(list->costume_hash).c_str(), get_face_name(list->face_hash).c_str(), get_face_name(it2->second).c_str());
					list->face_hash = it2->second;					
				}
			}
			
			if (list->unk_19 == 0)
			{
				next2 = list->next3;
				
				if (next2->unk_19 == 0)
				{
					list = next2;
					next2 = next2->next;
					
					while (next2->unk_19 == 0)
					{
						list = next2;
						next2 = next2->next;
					}
				}
				else
				{
					next2 = list->next2;
					
					while (next2->unk_19 == 0 && list == next2->next3)
					{
						list = next2;
						next2 = next2->next2;
					}
					
					list = next2;
				}
			}
			
		} // end while
	}
	
	//DPRINTF("End dump.\n");
}

PUBLIC void *load_costumes_patched(void *param)
{
	void *ret = load_costumes(param);
	patch_costumes_list();
	return ret;
}

// Not really layer2... but let's put it in this file
PUBLIC uint32_t get_related_face_patched(void *pthis, uint32_t costume, int idx)
{
	uint32_t ret = get_related_face(pthis, costume, idx);
	
	if (idx == 0)
	{	
		auto it = defaults_faces.find(costume);
		if (it != defaults_faces.end())
		{
			uint32_t face = it->second;
			if (face != ret)
			{
				//DPRINTF("Changing face from %s to %s (costume %s)\n", get_face_name(ret).c_str(), get_face_name(face).c_str(), get_costume_name(costume).c_str());
				mapped_face[face] = ret;
				ret = face;
			}			
		}
	}
		
	return ret;
}

PUBLIC uint32_t get_related_hair_patched(void *pthis, uint32_t costume, uint32_t face, int idx)
{
	uint32_t ret = get_related_hair(pthis, costume, face, idx);
	
	if (idx == 0)
	{	
		auto it = defaults_hairs.find(costume);
		if (it != defaults_hairs.end())
		{
			uint32_t hair = it->second;
			if (hair != ret)
			{
				//DPRINTF("Changing hair from %s to %s (costume %s)\n", get_hair_name(ret).c_str(), get_hair_name(hair).c_str(), get_costume_name(costume).c_str());
				ret = hair;
			}			
		}
		else if (ret == 0) // This is caused by defaults_faces. But 0 can't be returned or crash will happen
		{
			auto it2 = mapped_face.find(face);
			if (it2 != mapped_face.end())
			{
				uint32_t old_face = it2->second;
				//DPRINTF("Notice: had to change the face specified in get_related_hair from %s to %s\n", get_face_name(face).c_str(), get_face_name(old_face).c_str());
				ret = get_related_hair(pthis, costume, old_face, idx);
			}
		}
	}
	
	return ret;
}


/////
	
} // extern C

