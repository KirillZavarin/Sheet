#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    CellInterface* GetCellPtr(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

	// Можете дополнить ваш класс нужными полями и методами


private:
    //bool IsEmptyCell(Position pos) const;

    bool IsNeverSetCell(Position pos) const;

    Position VertexWithoutConsideringCell(Position pos) const;

    int extreme_col = -1;
    int extreme_row = -1;
    std::unordered_map<int, std::unordered_map<int, std::unique_ptr<CellInterface>>> sheet_;
	// Можете дополнить ваш класс нужными полями и методами
};