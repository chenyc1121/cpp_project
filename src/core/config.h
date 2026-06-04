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
    const char* titleDetail = nullptr;  // 点击 [i] 按钮显示的文字（空则回退到 titleBarText）

    //虚函数与多态格特有的
    int ratio;//人资产的比例
    int buy_ratio;//购入价的比例
    int rent_ratio;//收租的比例
    int buy_decay;//买入的价格减少
    int rent_decay;//收租的价格减少

    bool rentIsPureVirtual = false; // rent_price() 是否为纯虚函数(=0)
    bool rentIsNonVirtual  = false; // rent_price() 是否为非虚函数
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
// 角格：0(起点), 7(商店), 14(上机课), 21(29楼地下室)

const QVector<TileDef> BOARD_LAYOUT = {
    // 底部边 (索引 0-6)
    {TileType::START,       "起点",         ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "起点",     "经过或停留在起点可获得¥5000奖金。若掷骰后恰好停在起点，奖金翻倍至¥10000。"},
    {TileType::VIRTUALFUNC, "Buy",       ColorGroup::NONE,       1200,   80,     220,    600,    1400,   1700,   2000,   800,    "虚函数格", "class Buy:public VirtualfuncTile{\n int buy_price(){\nreturn get_price()*0.7+500;\n}\nint rent_price(){\nreturn get_rent();\n}\n}", "class VirtualfuncTile:{\nvirtual int buy_price(){\nreturn get_price();\n}\nvirtual int rent_price(){\nreturn get_rent();\n}\n}", 0, 70, 100, -500, 0, false, false},
    {TileType::PROPERTY,    "45甲",         ColorGroup::BROWN,      1200,   80,     220,    600,    1400,   1700,   2000,   800,    "棕色组",   "45甲 — 棕色组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部棕色组地产后租金翻倍。\n\n"},
    {TileType::QA,          "问答格",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "问答",     "回答一道C++面向对象选择题。\n答对后有概率获得效果卡（再丢一次骰子、万能骰子、虚函数卡、跳过卡）。\n答错无惩罚。\n\n用你的程设知识赢取优势！"},
    {TileType::PROPERTY,    "35楼",       ColorGroup::LIGHT_BLUE, 1600,   120,    360,    850,    1900,   2300,   2700,   1000,   "浅蓝组",   "35楼 — 浅蓝组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部浅蓝组地产后租金翻倍。\n\n"},
    {TileType::TAX,         "农园",       ColorGroup::NONE,       2000,   0,      0,      0,      0,      0,      0,      0,      "食堂",     "来都来了，吃了饭再走吧。停留在这个格子会花掉¥2000。"},
    {TileType::ITERATOR,    "三教",     ColorGroup::NONE,       2000,   250,    0,      0,      0,      0,      0,      0,      "迭代器格", "pos=0;\nauto now_pos=it+pos\n\n拥有迭代器卡时，可根据迭代器卡的类型和当前位置选择执行：\nnow_pos++| now_pos--|now_pos+=2| now_pos-=2", "vector<Tile> location = {\"三教\", \"二教\", \"理教\",\"一教\"};\nauto it = location.begin();"},

    // 左边边 (索引 7-13)
    {TileType::SHOP,        "麦叔的铺子",         ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "麦叔的铺子",     "欢迎来到麦叔的铺子！\n在这里可以用金币购买效果卡：\n• 再丢一次骰子 — ¥500\n• 万能骰子 — ¥1000\n• 虚函数卡 — ¥800\n• 跳过卡 — ¥1500"},
    {TileType::VIRTUALFUNC, "Pure",     ColorGroup::NONE,       2400,   200,    550,    1250,   2700,   3200,   3800,   1400,   "虚函数格", "class Pure:public VirtualfuncTile{\n int buy_price(){\nreturn get_price()*1.3;\n}\nint rent_price(){\nreturn get_rent()*1.5+get_guest_property()*0.1;\n}\n}", "class VirtualfuncTile:{\nvirtual int buy_price(){\nreturn get_price();\n}\nvirtual int rent_price()=0;\n}", 10, 130, 150, 0, 0, true, false},
    {TileType::PROPERTY,    "智华楼",       ColorGroup::LIGHT_BLUE, 1800,   140,    400,    950,    2100,   2500,   3000,   1000,   "浅蓝组",   "智华楼 — 浅蓝组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部浅蓝组地产后租金翻倍。\n\n"},
    {TileType::QA,          "问答格",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "问答",     "回答一道C++面向对象选择题。\n答对后有概率获得效果卡。\n答错无惩罚。"},
    {TileType::VIRTUALFUNC, "Rent",       ColorGroup::NONE,       1800,   140,    400,    950,    2100,   2500,   3000,   1000,   "虚函数格", "class Mix1:public VirtualfuncTile{\n int buy_price(){\nreturn get_price()-200;\n}\nint rent_price(){\nreturn get_rent()*0.6;\n}\n}", "class VirtualfuncTile:{\nvirtual int buy_price(){\nreturn get_price();\n}\nvirtual int rent_price(){\nreturn get_rent();\n}\n}", 0, 100, 60, 200, 0, false, false},
    {TileType::ITERATOR,    "二教",     ColorGroup::NONE,       1500,   250,    0,      0,      0,      0,      0,      0,      "迭代器格", "pos=1;\nauto now_pos=it+pos\n\n拥有迭代器卡时，可根据迭代器卡的类型和当前位置选择执行：\nnow_pos++| now_pos--|now_pos+=2| now_pos-=2", "vector<Tile> location = {\"三教\", \"二教\", \"理教\",\"一教\"};\nauto it = location.begin();"},
    {TileType::VIRTUALFUNC, "Mix1",     ColorGroup::NONE,       1600,   120,    360,    850,    1900,   2300,   2700,   1000,   "虚函数格", "class Mix1:public VirtualfuncTile{\n int buy_price(){\nreturn get_price()*1.1-300;\n}\nint rent_price(){\nreturn get_rent()*1.6;\n}\n}", "class VirtualfuncTile:{\nvirtual int buy_price(){\nreturn get_price();\n}\nint rent_price(){\nreturn get_rent();\n}\n}", 0, 110, 160, 300, 0, false, true},

    // 顶部边 (索引 14-20)
    {TileType::COMPUTER_LAB,"上机课",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "上机课",   "上机课！\n效果：停止一回合，同时回答一道C++选择题。\n答对必定获得一张效果卡。\n答错仅跳过下回合，无卡片奖励。"},
    {TileType::STATICVAL,   "静态成员变量·A", ColorGroup::YELLOW,   2000,   160,    440,    1050,   2300,   2700,   3200,   1000,   "静态变量", "int compute_cost(){\nif(count==3){\nreturn base_cost()+num_house*emptytile_cost\n}\n}\n\n", "class StaticvariableTile:{\nstatic int count;\nStaticvariableTile(){\ncount++;\n}\nint StaticvariableTile::count = 0;\n"},
    {TileType::STATICVAL,   "静态成员变量·B", ColorGroup::YELLOW,   2200,   180,    500,    1100,   2500,   3000,   3500,   1000,   "静态变量", "int compute_cost(){\nif(count==3){\nreturn base_cost()+num_house*emptytile_cost\n}\n}\n\n", "class StaticvariableTile:{\nstatic int count;\nStaticvariableTile(){\ncount++;\n}\nint StaticvariableTile::count = 0;\n"},
    {TileType::QA,          "问答格",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "问答",     "回答一道C++面向对象选择题。\n答对后有概率获得效果卡。\n答错无惩罚。"},
    {TileType::STATICVAL,   "静态成员变量·C", ColorGroup::YELLOW,   2400,   200,    550,    1250,   2700,   3200,   3800,   1000,   "静态变量", "int compute_cost(){\nif(count==3){\nreturn base_cost()+num_house*emptytile_cost\n}\n}\n\n", "class StaticvariableTile:{\nstatic int count;\nStaticvariableTile(){\ncount++;\n}\nint StaticvariableTile::count = 0;\n"},
    {TileType::TAX,         "燕南",       ColorGroup::NONE,       3000,   0,      0,      0,      0,      0,      0,      0,      "食堂",     "来都来了，吃了饭再走吧。停留在这个格子会花掉¥3000。"},
    {TileType::ITERATOR,    "理教",     ColorGroup::NONE,       2000,   250,    0,      0,      0,      0,      0,      0,      "迭代器格", "pos=2;\nauto now_pos=it+pos\n\n拥有迭代器卡时，可根据迭代器卡的类型和当前位置选择执行：\nnow_pos++| now_pos--|now_pos+=2| now_pos-=2", "vector<Tile> location = {\"三教\", \"二教\", \"理教\",\"一教\"};\nauto it = location.begin();"},
    // 右边边 (索引 21-27)
    {TileType::SHOP_ENTRANCE,"29楼地下室",    ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "29楼地下室", "前方是商店！\n你可以选择是否进入商店购买效果卡。\n进入：移动到商店格，浏览并购买卡片。\n不进入：停留原地，无效果。"},
    {TileType::PROPERTY,    "博雅塔",       ColorGroup::ORANGE,     2600,   220,    600,    1300,   2900,   3400,   4000,   1400,   "橙色组",   "博雅塔 — 橙色组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部橙色组地产后租金翻倍。\n\n"},
    {TileType::QA,          "问答格",       ColorGroup::NONE,       0,      0,      0,      0,      0,      0,      0,      0,      "问答",     "回答一道C++面向对象选择题。\n答对后有概率获得效果卡。\n答错无惩罚。"},
    {TileType::VIRTUALFUNC, "Mix2",     ColorGroup::NONE,       2000,   160,    440,    1050,   2300,   2700,   3200,   1200,   "虚函数格", "class Mix1:public VirtualfuncTile{\n int buy_price(){\nreturn get_price()*1.1;\n}\nint rent_price(){\nreturn get_rent()*0.9;\n}\n}", "class VirtualfuncTile:{\nvirtual int buy_price(){\nreturn get_price();\n}\nvirtual int rent_price(){\nreturn get_rent();\n}\n}", 0, 110, 90, 0, 0, false, false},
    {TileType::PROPERTY,    "未名湖",         ColorGroup::RED,        2800,   240,    700,    1500,   3100,   3600,   4300,   1600,   "红色组",   "未名湖 — 红色组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部红色组地产后租金翻倍。\n\n"},
    {TileType::ITERATOR,    "一教",   ColorGroup::NONE,       1500,   250,    0,      0,      0,      0,      0,      0,      "迭代器格", "pos=0;\nauto now_pos=it+pos\n\n拥有迭代器卡时，可根据迭代器卡的类型和当前位置选择执行：\nnow_pos++| now_pos--|now_pos+=2| now_pos-=2", "vector<Tile> location = {\"三教\", \"二教\", \"理教\",\"一教\"};\nauto it = location.rbegin();"},
    {TileType::PROPERTY,    "图书馆",   ColorGroup::RED,        3000,   260,    750,    1600,   3300,   3900,   4600,   1600,   "红色组",   "图书馆 — 红色组地产。\n购买后可收取租金。可建造房屋/旅馆；拥有全部红色组地产后租金翻倍。\n\n"},
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
