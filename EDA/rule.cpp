#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>

// ==========================================
// 1. 数据结构定义 (Data Structures)
// ==========================================

// 定义该规则作用的层级模式
enum class LayerMode {
    BOTH,
    ABOVE,
    BELOW
};

// 定义三大互斥判定分支 (参考理论分析)
enum class RuleBranch {
    UNSET,
    OPPOSITE_DIR, // 分支 A: 对称与方向传播 (OPPOSITE / WRONGDIRECTION)
    PROXIMITY,    // 分支 B: 经典邻域干涉 (WIDTH / SPANLENGTH ...)
    CONVEX        // 分支 C: 凸角与台阶 (CONVEXCORNERS ...)
};

class Lef58EnclosureEdgeRule {
public:
    // --- 基础公共属性 (Base Headers) ---
    std::string cutClass = "";      // 过孔类名，如果为空则代表无限制
    LayerMode layerMode = LayerMode::BOTH;
    double overhang = 0.0;          // 核心要求的包围量

    // 分支判定类型
    RuleBranch branchType = RuleBranch::UNSET;

    // --- 分支 A: OPPOSITE / WRONGDIRECTION ---
    bool isWrongDirection = false;
    bool hasOpposite = false;
    bool hasExceptEol = false;
    double exceptEolWidth = 0.0;
    bool hasNoConcaveCorner = false;
    double noConcaveWithin = 0.0;
    bool hasCutToBelowSpacing = false;
    double cutToBelowSpacing = 0.0;
    bool hasAboveMetal = false;
    double aboveMetalExtension = 0.0;

    // --- 分支 B: 经典邻域干涉 (Proximity) ---
    bool hasIncludeCorner = false;
    bool useSpanLength = false;     // false = WIDTH, true = SPANLENGTH
    bool hasBothWire = false;
    double minSize = 0.0;           // minWidth 或 minSpanLength
    double maxSize = -1.0;          // maxWidth 或 maxSpanLength (-1 表示无上限)
    
    bool hasParallel = false;
    double parallelLength = 0.0;
    
    double minWithin = 0.0;
    double maxWithin = -1.0;        // 如果仅提供了单值，则范围是 [0, maxWithin)

    bool hasExceptExtraCut = false;
    double extraCutWithin = -1.0;
    bool hasExceptTwoEdges = false;
    double exceptTwoEdgesWithin = -1.0;

    // --- 分支 C: 凸角与台阶 (Convex) ---
    bool hasConvexCorners = false;
    double convexLength = 0.0;
    double adjacentLength = 0.0;
    double convexParWithin = 0.0;
    double convexParLength = 0.0;

    // ==========================================
    // 2. 解析器方法 (Parser Method)
    // ==========================================
    
    // 核心装载函数：将 vector<string> strs 的信息存入 Class 中
    bool buildFromTokens(const std::vector<std::string>& tokens) {
        if (tokens.empty() || tokens[0] != "ENCLOSUREEDGE") return false;

        bool overhangParsed = false; // 用于标记是否已经解析到了基准 overhang 数值

        for (size_t i = 1; i < tokens.size(); ++i) {
            const std::string& t = tokens[i];

            // -- 公共头部解析 --
            if (t == "CUTCLASS") {
                if (i + 1 < tokens.size()) cutClass = tokens[++i];
            } else if (t == "ABOVE") {
                layerMode = LayerMode::ABOVE;
            } else if (t == "BELOW") {
                layerMode = LayerMode::BELOW;
            } 
            // 匹配悬空数字（通常就是 overhang 参数）
            else if (!overhangParsed && isNumber(t)) {
                overhang = std::stod(t);
                overhangParsed = true;
            }
            // -- 分支 A 解析 --
            else if (t == "OPPOSITE") {
                branchType = RuleBranch::OPPOSITE_DIR;
                hasOpposite = true;
            } else if (t == "WRONGDIRECTION") {
                branchType = RuleBranch::OPPOSITE_DIR;
                isWrongDirection = true;
            } else if (t == "EXCEPTEOL") {
                hasExceptEol = true;
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) exceptEolWidth = std::stod(tokens[++i]);
            } else if (t == "NOCONCAVECORNER") {
                hasNoConcaveCorner = true;
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) noConcaveWithin = std::stod(tokens[++i]);
            } else if (t == "CUTTOBELOWSPACING") {
                hasCutToBelowSpacing = true;
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) cutToBelowSpacing = std::stod(tokens[++i]);
            } else if (t == "ABOVEMETAL") {
                hasAboveMetal = true;
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) aboveMetalExtension = std::stod(tokens[++i]);
            }
            // -- 分支 B 解析 --
            else if (t == "INCLUDECORNER") {
                hasIncludeCorner = true;
            } else if (t == "WIDTH" || t == "SPANLENGTH") {
                branchType = RuleBranch::PROXIMITY;
                useSpanLength = (t == "SPANLENGTH");
                
                // 向前看 (Look-ahead) 判定是否有 BOTHWIRE
                if (i + 1 < tokens.size() && tokens[i+1] == "BOTHWIRE") {
                    hasBothWire = true;
                    i++;
                }
                
                // 必定有一个 min 值
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) minSize = std::stod(tokens[++i]);
                
                // 尝试再向前看一步，如果还是数字，说明有 max 值
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) maxSize = std::stod(tokens[++i]);
                
            } else if (t == "PARALLEL") {
                hasParallel = true;
                // 注意：在分支B中是 PARALLEL parLength; 在分支C中是 PARALLEL parWithin LENGTH length
                if (branchType == RuleBranch::CONVEX) {
                    if (i + 1 < tokens.size() && isNumber(tokens[i+1])) convexParWithin = std::stod(tokens[++i]);
                } else {
                    if (i + 1 < tokens.size() && isNumber(tokens[i+1])) parallelLength = std::stod(tokens[++i]);
                }
            } else if (t == "WITHIN") {
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) {
                    minWithin = std::stod(tokens[++i]);
                }
                // 向前看判断是单值区间 (WITHIN parWithin) 还是双值区间 (WITHIN min max)
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) {
                    maxWithin = std::stod(tokens[++i]);
                } else {
                    // 若只有一个值，那它实际上是 maxWithin 限制 (即距离 < parWithin)
                    maxWithin = minWithin;
                    minWithin = 0.0;
                }
            } else if (t == "EXCEPTEXTRACUT") {
                hasExceptExtraCut = true;
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) extraCutWithin = std::stod(tokens[++i]);
            } else if (t == "EXCEPTTWOEDGES") {
                hasExceptTwoEdges = true;
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) exceptTwoEdgesWithin = std::stod(tokens[++i]);
            }
            // -- 分支 C 解析 --
            else if (t == "CONVEXCORNERS") {
                branchType = RuleBranch::CONVEX;
                hasConvexCorners = true;
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) convexLength = std::stod(tokens[++i]);
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) adjacentLength = std::stod(tokens[++i]);
            } else if (t == "LENGTH") {
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) convexParLength = std::stod(tokens[++i]);
            }
        }
        return true;
    }

    // ==========================================
    // 3. DRC 引擎读取校验接口 (Checker Mock)
    // ==========================================
    // 后续在别的地方获取到这个 rule 后，调用此方法检查是否违规
    bool isOverhangRequired(double wireSize, double parallelDist, double parallelOverlap, bool hasSharedCut) const {
        // 第一关过滤：如果没有进入分支B，这里不写复杂逻辑
        if (branchType != RuleBranch::PROXIMITY) return false;

        // 检查尺寸触发
        bool sizeTriggered = (wireSize >= minSize) && (maxSize == -1.0 || wireSize < maxSize);
        if (!sizeTriggered) return false;

        // 检查距离与重叠长度
        bool distTriggered = (parallelDist >= minWithin && parallelDist < maxWithin);
        bool overlapTriggered = (parallelOverlap >= parallelLength);

        if (distTriggered && overlapTriggered) {
            // 检查是否有豁免
            if (hasExceptExtraCut && hasSharedCut /* 且在 extraCutWithin 范围内 */) {
                return false; // 被多孔规则豁免
            }
            return true; // 强制触发！
        }
        return false;
    }

    // 调试打印工具
    void debugPrint() const {
        std::cout << "--- 规则解析结果 ---\n";
        std::cout << "Cut Class: " << (cutClass.empty() ? "NONE" : cutClass) << "\n";
        std::cout << "Overhang: " << overhang << "\n";
        
        if (branchType == RuleBranch::PROXIMITY) {
            std::cout << "Branch: PROXIMITY (邻域干涉)\n";
            std::cout << "Size Range: [" << minSize << ", " << (maxSize == -1 ? "INF" : std::to_string(maxSize)) << ")\n";
            std::cout << "Distance WITHIN: [" << minWithin << ", " << maxWithin << ")\n";
            std::cout << "EXCEPTEXTRACUT Allowed: " << (hasExceptExtraCut ? "YES" : "NO") << "\n";
        }
        std::cout << "--------------------\n";
    }

private:
    // 工具方法：判断字符串是否代表数字
    bool isNumber(const std::string& str) const {
        if (str.empty()) return false;
        char c = str[0];
        return std::isdigit(c) || c == '-' || c == '.';
    }
};

// ==========================================
// 4. 使用示例 (Usage Example)
// ==========================================
int main() {
    // 假设你从 LEF 文件中按空格拆分出了这样一个 vector
    std::vector<std::string> ruleStrs = {
        "ENCLOSUREEDGE", "CUTCLASS", "VIA12_LONG", "ABOVE", "0.045",
        "SPANLENGTH", "0.1", "0.3",
        "PARALLEL", "0.05",
        "WITHIN", "0.02", "0.08",
        "EXCEPTEXTRACUT", "0.06"
    };

    // 1. 创建对象并存入信息
    Lef58EnclosureEdgeRule myRule;
    if (myRule.buildFromTokens(ruleStrs)) {
        std::cout << "规则解析成功！\n";
    }

    // 2. 打印看看类里存入了什么
    myRule.debugPrint();

    // 3. 在别的地方进行 Check (模拟物理设计验证)
    double targetWireSize = 0.15;    // 金属跨长0.15 (落入0.1-0.3区间)
    double neighborDist = 0.05;      // 邻居距离0.05 (落入0.02-0.08区间)
    double neighborOverlap = 0.10;   // 平行重叠0.10 (大于阈值0.05)
    bool hasSharedCutNearby = false; // 附近没有别的孔

    bool needStrictOverhang = myRule.isOverhangRequired(targetWireSize, neighborDist, neighborOverlap, hasSharedCutNearby);
    
    std::cout << "DRC 判定: " << (needStrictOverhang ? "违规，需要严格包围量！" : "通过，使用普通包围量。") << "\n";

    return 0;
}
