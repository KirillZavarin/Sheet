#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <set>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression): ast_(ParseFormulaAST(expression)) {       
    }

    std::vector<Position> GetReferencedCells() const override {
        auto cells = ast_.GetCells();
        auto set_cell = std::set<Position>(cells.begin(), cells.end());
        return std::vector<Position>(set_cell.begin(), set_cell.end());
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            auto func = [&sheet](Position pos) 
            {
                try {
                    return (sheet.GetCell(pos) != nullptr) ? sheet.GetCell(pos)->GetValue() : CellInterface::Value(0.);
                }
                catch (...) {

                    throw FormulaError(FormulaError::Category::Ref);
                }
            };

            return ast_.Execute(func);
        }
        catch(FormulaError& fe) {
            return fe;
        }
    }

    std::string GetExpression() const override {
        std::stringstream ss;
        ast_.PrintFormula(ss);
        return ss.str();
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("Bad formula)");
    }
}