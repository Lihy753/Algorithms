#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>
、、、、
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

// ==========================================
// 1. 单条规则语句的数据结构 (Single Statement)
// ==========================================
class Lef58EnclosureEdgeStatement {
public:
    // --- 基础公共属性 ---
    std::string cutClass = "";      
    LayerMode layerMode = LayerMode::BOTH;
    double overhang = 0.0;          

    // 分支判定类型
    RuleBranch branchType = RuleBranch::UNSET;

    // --- 分支 A 属性 (总开关: OPPOSITE) ---
    bool hasOpposite = false;      // 只要进入分支A，此项必然为 true
    
    // OPPOSITE 下的内层互斥子分支 1: 强制非首选方向约束
    bool isWrongDirection = false; 
    
    // OPPOSITE 下的内层互斥子分支 2: 对称方向防护 (可附带以下可选条件)
    bool hasExceptEol = false;
    double exceptEolWidth = 0.0;
    bool hasNoConcaveCorner = false;
    double noConcaveWithin = 0.0;
    bool hasCutToBelowSpacing = false;
    double cutToBelowSpacing = 0.0;
    bool hasAboveMetal = false;
    double aboveMetalExtension = 0.0;

    // --- 分支 B 属性 ---
    bool hasIncludeCorner = false;
    bool useSpanLength = false;     
    bool hasBothWire = false;
    double minSize = 0.0;           
    double maxSize = -1.0;          
    bool hasParallel = false;
    double parallelLength = 0.0;
    double minWithin = 0.0;
    double maxWithin = -1.0;        
    bool hasExceptExtraCut = false;
    double extraCutWithin = -1.0;
    bool hasExceptTwoEdges = false;
    double exceptTwoEdgesWithin = -1.0;

    // --- 分支 C 属性 ---
    bool hasConvexCorners = false;
    double convexLength = 0.0;
    double adjacentLength = 0.0;
    double convexParWithin = 0.0;
    double convexParLength = 0.0;
};

// ==========================================
// 2. 整个属性的管理者类 (The Property Container)
// ==========================================
class Lef58EnclosureEdgeProperty {
public:
    // 核心序列：这里完美解决了你说的“能出现很多次”的问题！
    std::vector<Lef58EnclosureEdgeStatement> statements;

    // 状态机解析器：负责遍历你的 vector<string> strs
    bool parseTokens(const std::vector<std::string>& tokens) {
        if (tokens.empty()) return false;

        Lef58EnclosureEdgeStatement currentStmt;
        bool inStatement = false;
        bool overhangParsed = false;

        for (size_t i = 0; i < tokens.size(); ++i) {
            const std::string& t = tokens[i];

            // 遇到 ENCLOSUREEDGE 关键字，代表一个新语句（序列元素）的开始
            if (t == "ENCLOSUREEDGE") {
                currentStmt = Lef58EnclosureEdgeStatement(); // 初始化一个干净的新对象
                inStatement = true;
                overhangParsed = false;
                continue;
            }

            // 遇到分号 ; 代表当前语句结束，将其塞入序列并准备迎接下一个
            if (t == ";") {
                if (inStatement) {
                    statements.push_back(currentStmt);
                    inStatement = false;
                }
                continue;
            }

            // 如果不在任何语句解析状态中，忽略杂音
            if (!inStatement) continue;

            // --- 下面就是单纯给 currentStmt 填值的逻辑 ---
            if (t == "CUTCLASS" && i + 1 < tokens.size()) {
                currentStmt.cutClass = tokens[++i];
            } else if (t == "ABOVE") {
                currentStmt.layerMode = LayerMode::ABOVE;
            } else if (t == "BELOW") {
                currentStmt.layerMode = LayerMode::BELOW;
            } else if (!overhangParsed && isNumber(t)) {
                currentStmt.overhang = std::stod(t);
                overhangParsed = true;
            }
            // 分支 A: 必须以 OPPOSITE 为大前提
            else if (t == "OPPOSITE") {
                currentStmt.branchType = RuleBranch::OPPOSITE_DIR;
                currentStmt.hasOpposite = true;
            } else if (t == "WRONGDIRECTION") {
                // 这是隶属于 OPPOSITE 的子选项
                currentStmt.isWrongDirection = true;
            } else if (t == "EXCEPTEOL" && i + 1 < tokens.size() && isNumber(tokens[i+1])) {
                currentStmt.hasExceptEol = true;
                currentStmt.exceptEolWidth = std::stod(tokens[++i]);
            }
            // 分支 B
            else if (t == "WIDTH" || t == "SPANLENGTH") {
                currentStmt.branchType = RuleBranch::PROXIMITY;
                currentStmt.useSpanLength = (t == "SPANLENGTH");
                if (i + 1 < tokens.size() && tokens[i+1] == "BOTHWIRE") {
                    currentStmt.hasBothWire = true; i++;
                }
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) currentStmt.minSize = std::stod(tokens[++i]);
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) currentStmt.maxSize = std::stod(tokens[++i]);
            } else if (t == "PARALLEL") {
                currentStmt.hasParallel = true;
                if (currentStmt.branchType == RuleBranch::CONVEX && i + 1 < tokens.size() && isNumber(tokens[i+1])) {
                    currentStmt.convexParWithin = std::stod(tokens[++i]);
                } else if (i + 1 < tokens.size() && isNumber(tokens[i+1])) {
                    currentStmt.parallelLength = std::stod(tokens[++i]);
                }
            } else if (t == "WITHIN") {
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) currentStmt.minWithin = std::stod(tokens[++i]);
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) {
                    currentStmt.maxWithin = std::stod(tokens[++i]);
                } else {
                    currentStmt.maxWithin = currentStmt.minWithin;
                    currentStmt.minWithin = 0.0;
                }
            }
            // 分支 C
            else if (t == "CONVEXCORNERS") {
                currentStmt.branchType = RuleBranch::CONVEX;
                currentStmt.hasConvexCorners = true;
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) currentStmt.convexLength = std::stod(tokens[++i]);
                if (i + 1 < tokens.size() && isNumber(tokens[i+1])) currentStmt.adjacentLength = std::stod(tokens[++i]);
            }
        }
        return !statements.empty();
    }

    // ==========================================
    // 3. DRC 引擎读取校验接口 (Checker Mock)
    // ==========================================
    // 外部调用时，会遍历整个序列，看当前物理状态命中了哪一条语句
    bool checkViolation(double wireSize, double neighborDist, double parallelOverlap) const {
        for (const auto& stmt : statements) {
            // 这里仅以检查分支 B 为例
            if (stmt.branchType == RuleBranch::PROXIMITY) {
                bool sizeOk = (wireSize >= stmt.minSize) && (stmt.maxSize == -1.0 || wireSize < stmt.maxSize);
                bool distOk = (neighborDist >= stmt.minWithin && neighborDist < stmt.maxWithin);
                bool overlapOk = (parallelOverlap >= stmt.parallelLength);
                
                // 如果命中了序列中的任意一条触发条件
                if (sizeOk && distOk && overlapOk) {
                    return true; // 强制要求严格的 overhang
                }
            }
        }
        return false;
    }

private:
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
    // 模拟你解析出来的 strs，注意这里面包含了 两个 ENCLOSUREEDGE 语句（序列）！
    std::vector<std::string> myStrs = {
        // 第一条语句 (针对 VIA12_LONG)
        "ENCLOSUREEDGE", "CUTCLASS", "VIA12_LONG", "ABOVE", "0.045",
        "SPANLENGTH", "0.1", "0.3", "PARALLEL", "0.05", "WITHIN", "0.02", "0.08", ";",
        
        // 第二条语句 (针对普通 VIA)
        "ENCLOSUREEDGE", "ABOVE", "0.02",
        "WIDTH", "0.05", "0.1", "PARALLEL", "0.03", "WITHIN", "0.05", ";"
    };

    Lef58EnclosureEdgeProperty enclosureProperty;
    
    // 解析并存入信息
    if (enclosureProperty.parseTokens(myStrs)) {
        std::cout << "解析成功！共加载了 " << enclosureProperty.statements.size() << " 条规则序列。\n";
    }

    // 物理设计验证
    bool isViolated = enclosureProperty.checkViolation(0.15, 0.05, 0.10);
    std::cout << "DRC 判定: " << (isViolated ? "触发严格约束" : "安全") << "\n";

    return 0;
}
