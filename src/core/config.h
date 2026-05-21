#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QVector>

// ==================== 游戏全局配置 ====================

// 初始资金
constexpr int INITIAL_MONEY = 20000;
// 经过起点获得的奖金
constexpr int START_BONUS = 5000;

// 地块类型枚举
enum class TileType {
    START,          // 起点
    PROPERTY,       // 可购买地产
    QA,             // 问答格（替代机会/命运）
    TAX,            // 税收
    SHOP,           // 商店（替代监狱）
    COMPUTER_LAB,   // 上机课（替代免费停留）
    SHOP_ENTRANCE,  // 商店入口（替代送往监狱）
    UTILITY,        // 公共设施
    RAILROAD,       // 铁路/车站
    STATICVAL,      // 静态成员变量
    VIRTUALFUNC,    // 虚函数
    ITERATOR        // 迭代器格
};

// 颜色组枚举（用于地产租金加成）
enum class ColorGroup {
    BROWN,      // 棕色 
    LIGHT_BLUE, // 浅蓝
    PINK,       // 粉色
    ORANGE,     // 橙色
    RED,        // 红色
    YELLOW,     // 黄色
    GREEN,      // 绿色
    DEEP_BLUE,  // 深蓝 — 最贵
    NONE        // 非地产类
};

// 地块定义结构（用于在 config.h 中描述整个棋盘）
struct TileDef {
    TileType type;
    QString name;              // 地块名称
    ColorGroup group;          // 颜色组（仅 PROPERTY 有效）
    int price;                 // 购买价格 / 税收金额
    int baseRent;              // 基础租金
    int rentWith1House;        // 1栋房子租金
    int rentWith2House;        // 2栋房子租金
    int rentWith3House;        // 3栋房子租金
    int rentWith4House;        // 4栋房子租金
    int rentWithHotel;         // 旅馆租金
    int houseCost;             // 建一栋房子的费用
    const char* titleBarText;  // 标题栏显示文字
    const char* infoText;      // 格子详情按钮弹窗文字

    //虚函数与多态格特有的
    int ratio;//人资产的比例
    int buy_ratio;//购入价的比例
    int rent_ratio;//收租的比例
    int buy_decay;//买入的价格减少
    int rent_decay;//收租的价格减少
};

// ==================== 效果卡价格常量 ====================
constexpr int CARD_PRICE_ROLL_AGAIN = 500;
constexpr int CARD_PRICE_UNIVERSAL_DICE = 1000;
constexpr int CARD_PRICE_VIRTUAL_FUNCTION = 800;
constexpr int CARD_PRICE_SKIP_EFFECT = 1500;
constexpr int CARD_PRICE_ITERATOR = 600;

// QA格答对后获得效果卡的概率（百分比）
constexpr int QA_CARD_CHANCE_PERCENT = 65;

// ==================== 棋盘布局定义（28格） ====================
// 布局：4个角格 + 每条边6个中间格 = 28格
// 角格：0(起点), 7(商店), 14(上机课), 21(商店入口)

const QVector<TileDef> BOARD_LAYOUT = {
    // 底部边 (索引 0-6)：起点 → 亚洲区 → 问答格 → 税收 → 铁路
    {TileType::START,       "起点",         ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "起点",     "经过或停留在起点可获得¥5000奖金。若掷骰后恰好停在起点，奖金翻倍至¥10000。"},
    {TileType::VIRTUALFUNC, "虚函数",       ColorGroup::NONE,       1200,   80,     220,    600,    1400,   1700,   2000,   800,    "虚函数格", "虚函数格 — 使用虚函数卡可选择基类或派生类行为。\n\n基类价格：¥1200（固定）\n派生类价格：¥840（70%）\n基类租金：标准\n派生类租金：130% + 被收租者资产的5%\n\n便宜买进，高租金收入！", 5, 70, 130, 0, 0},
    {TileType::PROPERTY,    "旺角",         ColorGroup::BROWN,      1200,   80,     220,    600,    1400,   1700,   2000,   800,    "棕色组",   "旺角 — 棕色组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部棕色组地产后租金翻倍。\n\n租金表：裸地¥80 | 1栋¥220 | 2栋¥600 | 3栋¥1400 | 4栋¥1700 | 旅馆¥2000\n建房费：¥800/栋"},
    {TileType::QA,          "问答格",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "问答",     "回答一道C++面向对象选择题。\n答对后有概率获得效果卡（再丢一次骰子、万能骰子、虚函数卡、跳过卡）。\n答错无惩罚。\n\n用你的C++知识赢取优势！"},
    {TileType::PROPERTY,    "浅水湾",       ColorGroup::LIGHT_BLUE, 1600,   120,    360,    850,    1900,   2300,   2700,   1000,   "浅蓝组",   "浅水湾 — 浅蓝组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部浅蓝组地产后租金翻倍。\n\n租金表：裸地¥120 | 1栋¥360 | 2栋¥850 | 3栋¥1900 | 4栋¥2300 | 旅馆¥2700\n建房费：¥1000/栋"},
    {TileType::TAX,         "所得税",       ColorGroup::NONE,       2000,   0,      0,      0,      0,      0,      0,      0,      "税务",     "停留在此格需缴纳所得税¥2000。"},
    {TileType::ITERATOR,    "西九龙站",     ColorGroup::NONE,       2000,   250,    0,      0,      0,      0,      0,      0,      "迭代器格", "迭代器格 — 拥有多个迭代器格可收取更高租金。\n\n拥有迭代器卡时，可在迭代器格之间传送：\n++（前进1格）| --（后退1格）\n+=2（前进2格）| -=2（后退2格）\n\n租金：1个=¥250 | 2个=¥500 | 3个=¥1000 | 4个=¥2000"},

    // 左边边 (索引 7-13)：商店 → 地产 → 问答格 → 地产 → 铁路 → 地产
    {TileType::SHOP,        "商店",         ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "商店",     "欢迎来到商店！\n在这里可以用金币购买效果卡：\n• 再丢一次骰子 — ¥500\n• 万能骰子 — ¥1000\n• 虚函数卡 — ¥800\n• 跳过卡 — ¥1500"},
    {TileType::VIRTUALFUNC, "纯虚函数",     ColorGroup::NONE,       2400,   200,    550,    1250,   2700,   3200,   3800,   1400,   "虚函数格", "纯虚函数格 — 不能直接实例化，代价高昂。\n\n基类价格：¥2400（固定）\n派生类价格：¥3120（130%）\n基类租金：标准\n派生类租金：150% + 被收租者资产的8%\n\n极贵买入，极高租金收入！", 8, 130, 150, 0, 0},
    {TileType::PROPERTY,    "淮海路",       ColorGroup::LIGHT_BLUE, 1800,   140,    400,    950,    2100,   2500,   3000,   1000,   "浅蓝组",   "淮海路 — 浅蓝组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部浅蓝组地产后租金翻倍。\n\n租金表：裸地¥140 | 1栋¥400 | 2栋¥950 | 3栋¥2100 | 4栋¥2500 | 旅馆¥3000\n建房费：¥1000/栋"},
    {TileType::QA,          "问答格",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "问答",     "回答一道C++面向对象选择题。\n答对后有概率获得效果卡。\n答错无惩罚。"},
    {TileType::VIRTUALFUNC, "虚析构",       ColorGroup::NONE,       1800,   140,    400,    950,    2100,   2500,   3000,   1000,   "虚函数格", "虚析构格 — 安全的投资选择。\n\n基类价格：¥1800（固定）\n派生类价格：¥1600（100%-200）\n基类租金：标准\n派生类租金：60%（低风险低回报）\n\n中价买入，低租金收入但稳定。", 0, 100, 60, 200, 0},
    {TileType::ITERATOR,    "电力公司",     ColorGroup::NONE,       1500,   250,    0,      0,      0,      0,      0,      0,      "迭代器格", "迭代器格 — 拥有多个迭代器格可收取更高租金。\n\n拥有迭代器卡时，可在迭代器格之间传送。\n\n租金：1个=¥250 | 2个=¥500 | 3个=¥1000 | 4个=¥2000"},
    {TileType::VIRTUALFUNC, "动态绑定",     ColorGroup::NONE,       1600,   120,    360,    850,    1900,   2300,   2700,   1000,   "虚函数格", "动态绑定格 — 运行时才确定价格。\n\n基类价格：¥1600（固定）\n派生类价格：¥960（60%）— 极便宜！\n基类租金：标准\n派生类租金：160% + 被收租者资产的10%\n\n陷阱格：买得便宜，付租时倾家荡产！", 10, 60, 160, 0, 0},

    // 顶部边 (索引 14-20)：上机课 → 地产 → 问答格 → 地产 → 税收 → 铁路
    {TileType::COMPUTER_LAB,"上机课",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "上机课",   "上机课（计算机实验课）！\n效果：停止一回合，同时回答一道C++选择题。\n答对必定获得一张效果卡。\n答错仅跳过下回合，无卡片奖励。"},
    {TileType::STATICVAL,   "静态成员变量·A", ColorGroup::YELLOW,   2000,   160,    440,    1050,   2300,   2700,   3200,   1000,   "静态变量", "静态成员变量格 — 类的所有实例共享同一份数据。\n\n属于黄色组（静态成员变量组）。\n集齐全部3个同组格后，租金额外增加空地基础租金。\n\n租金表：裸地¥160 | 1栋¥440 | 2栋¥1050 | 3栋¥2300 | 4栋¥2700 | 旅馆¥3200\n建房费：¥1000/栋"},
    {TileType::STATICVAL,   "静态成员变量·B", ColorGroup::YELLOW,   2200,   180,    500,    1100,   2500,   3000,   3500,   1000,   "静态变量", "静态成员变量格 — 类的所有实例共享同一份数据。\n\n属于黄色组（静态成员变量组）。\n集齐全部3个同组格后，租金额外增加空地基础租金。\n\n租金表：裸地¥180 | 1栋¥500 | 2栋¥1100 | 3栋¥2500 | 4栋¥3000 | 旅馆¥3500\n建房费：¥1000/栋"},
    {TileType::QA,          "问答格",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "问答",     "回答一道C++面向对象选择题。\n答对后有概率获得效果卡。\n答错无惩罚。"},
    {TileType::STATICVAL,   "静态成员变量·C", ColorGroup::YELLOW,   2400,   200,    550,    1250,   2700,   3200,   3800,   1000,   "静态变量", "静态成员变量格 — 类的所有实例共享同一份数据。\n\n属于黄色组（静态成员变量组）。\n集齐全部3个同组格后，租金额外增加空地基础租金。\n\n租金表：裸地¥200 | 1栋¥550 | 2栋¥1250 | 3栋¥2700 | 4栋¥3200 | 旅馆¥3800\n建房费：¥1000/栋"},
    {TileType::TAX,         "奢侈税",       ColorGroup::NONE,       3000,   0,      0,      0,      0,      0,      0,      0,      "税务",     "停留在此格需缴纳奢侈税¥3000。"},
    {TileType::ITERATOR,    "北京南站",     ColorGroup::NONE,       2000,   250,    0,      0,      0,      0,      0,      0,      "迭代器格", "迭代器格 — 拥有多个迭代器格可收取更高租金。\n\n拥有迭代器卡时，可在迭代器格之间传送。\n\n租金：1个=¥250 | 2个=¥500 | 3个=¥1000 | 4个=¥2000"},

    // 右边边 (索引 21-27)：商店入口 → 地产 → 问答格 → 地产 → 地产 → 铁路
    {TileType::SHOP_ENTRANCE,"商店入口",    ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "商店入口", "前方是商店！\n你可以选择是否进入商店购买效果卡。\n进入：移动到商店格，浏览并购买卡片。\n不进入：停留原地，无效果。"},
    {TileType::PROPERTY,    "春熙路",       ColorGroup::ORANGE,     2600,   220,    600,    1300,   2900,   3400,   4000,   1400,   "橙色组",   "春熙路 — 橙色组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部橙色组地产后租金翻倍。\n\n租金表：裸地¥220 | 1栋¥600 | 2栋¥1300 | 3栋¥2900 | 4栋¥3400 | 旅馆¥4000\n建房费：¥1400/栋"},
    {TileType::QA,          "问答格",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "问答",     "回答一道C++面向对象选择题。\n答对后有概率获得效果卡。\n答错无惩罚。"},
    {TileType::VIRTUALFUNC, "重载决议",     ColorGroup::NONE,       2000,   160,    440,    1050,   2300,   2700,   3200,   1200,   "虚函数格", "重载决议格 — 编译器在多个候选函数中选择最佳匹配。\n\n基类价格：¥2000（固定）\n派生类价格：¥1700（90%-100）\n基类租金：标准\n派生类租金：90% + 被收租者资产的3% - 50\n\n均衡型：买卖都略低于标准。", 3, 90, 90, 100, 50},
    {TileType::PROPERTY,    "锦里",         ColorGroup::RED,        2800,   240,    700,    1500,   3100,   3600,   4300,   1600,   "红色组",   "锦里 — 红色组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部红色组地产后租金翻倍。\n\n租金表：裸地¥240 | 1栋¥700 | 2栋¥1500 | 3栋¥3100 | 4栋¥3600 | 旅馆¥4300\n建房费：¥1600/栋"},
    {TileType::ITERATOR,    "自来水公司",   ColorGroup::NONE,       1500,   250,    0,      0,      0,      0,      0,      0,      "迭代器格", "迭代器格 — 拥有多个迭代器格可收取更高租金。\n\n拥有迭代器卡时，可在迭代器格之间传送。\n\n租金：1个=¥250 | 2个=¥500 | 3个=¥1000 | 4个=¥2000"},
    {TileType::PROPERTY,    "武侯祠大街",   ColorGroup::RED,        3000,   260,    750,    1600,   3300,   3900,   4600,   1600,   "红色组",   "武侯祠大街 — 红色组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部红色组地产后租金翻倍。\n\n租金表：裸地¥260 | 1栋¥750 | 2栋¥1600 | 3栋¥3300 | 4栋¥3900 | 旅馆¥4600\n建房费：¥1600/栋"},
};

constexpr int BOARD_SIZE = 28;

// 颜色组对应的中文名称
inline QString colorGroupName(ColorGroup g) {
    switch (g) {
    case ColorGroup::BROWN:       return "棕色组";
    case ColorGroup::LIGHT_BLUE:  return "浅蓝组";
    case ColorGroup::PINK:        return "粉色组";
    case ColorGroup::ORANGE:      return "橙色组";
    case ColorGroup::RED:         return "红色组";
    case ColorGroup::YELLOW:      return "黄色组";
    case ColorGroup::GREEN:       return "绿色组";
    case ColorGroup::DEEP_BLUE:   return "深蓝组";
    default: return "";
    }
}

// 每个颜色组包含的地产数量（用于判断是否成套）
inline int colorGroupSize(ColorGroup g) {
    switch (g) {
    case ColorGroup::BROWN:
    case ColorGroup::DEEP_BLUE: return 2;
    default: return 3;
    }
}

#endif // CONFIG_H
