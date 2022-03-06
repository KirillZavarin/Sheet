#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (pos.IsValid()) {
        if (pos.col > extreme_col) {
            extreme_col = pos.col;
        }
        if (pos.row > extreme_row) {
            extreme_row = pos.row;
        }

        if (IsNeverSetCell(pos)) {
            (sheet_[pos.col][pos.row] = std::unique_ptr<CellInterface>(new Cell(*this)))->Set(text);
        }
        else {
            sheet_[pos.col][pos.row]->Set(text);
        }
        
        return;
    }
    throw InvalidPositionException("No valid position");
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (pos.IsValid()){
        return IsNeverSetCell(pos) ? nullptr : sheet_.at(pos.col).at(pos.row).get();
    }
    else {
        throw InvalidPositionException("No valid position");
    }
}
CellInterface* Sheet::GetCell(Position pos) {
    if (pos.IsValid()) {
        return IsNeverSetCell(pos) ? nullptr : sheet_.at(pos.col).at(pos.row).get();
    }
    else {
        throw InvalidPositionException("No valid position");
    }
}

CellInterface* Sheet::GetCellPtr(Position pos) {
    if (pos.IsValid()) {
        if (IsNeverSetCell(pos)) {
            SetCell(pos, "");
        }
        return sheet_.at(pos.col).at(pos.row).get();
    }
    else {
        throw InvalidPositionException("No valid position");
    }
}

void Sheet::ClearCell(Position pos) {
    if (pos.IsValid()) {
        if (IsNeverSetCell(pos)) {
            return;
        }
        auto vertex = VertexWithoutConsideringCell(pos);
        extreme_col = vertex.col;
        extreme_row = vertex.row;

        sheet_.at(pos.col).erase(pos.row);
        if (sheet_.at(pos.col).size() == 0) {
            sheet_.erase(pos.col);
        }
    }
    else {
        throw InvalidPositionException("No valid position");
    }
}

Size Sheet::GetPrintableSize() const {
    Size result;
    result.cols = extreme_col + 1;
    result.rows = extreme_row + 1;
    return result;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int row = 0; row <= extreme_row; row++) {
        for (int col = 0; col <= extreme_col; col++) {
            Position pos;
            pos.col = col;
            pos.row = row;

            if (!IsNeverSetCell(pos)) {
                auto val = sheet_.at(col).at(row)->GetValue();
                if (val.index() == 0) {
                    output << std::get<std::string>(val);
                }
                if (val.index() == 1) {
                    output << std::get<double>(val);
                }
                if (val.index() == 2) {
                    output << std::get<FormulaError>(val);
                }
            }
            if (col != extreme_row) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int row = 0; row <= extreme_row; row++) {
        for (int col = 0; col <= extreme_col; col++) {
            Position pos;
            pos.col = col;
            pos.row = row;
        
            if (!IsNeverSetCell(pos)) {
                output << sheet_.at(col).at(row)->GetText();
            }
            if (col != extreme_row) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

bool Sheet::IsNeverSetCell(Position pos) const {
    if (sheet_.find(pos.col) == sheet_.end()) {
        return true;
    }
    if (sheet_.at(pos.col).find(pos.row) == sheet_.at(pos.col).end()) {
        return true;
    }
    return false;
}

Position Sheet::VertexWithoutConsideringCell(Position pos) const {
    if (IsNeverSetCell(pos)) {
        return Position{ extreme_row, extreme_col };
    }

    Position result{ extreme_row, extreme_col };
    if (pos.col == extreme_col && sheet_.at(pos.col).size() == 1) {
        int new_extreme_col = -1;
        for (const auto& [number, col] : sheet_) {
            if (number == extreme_col) {
                continue;
            }

            if (number > new_extreme_col) {
                new_extreme_col = number;
            }
        }
        result.col = new_extreme_col;
    }

    if (pos.row == extreme_row) {
        int number_element_in_row = 0;
        int new_extreme_row = -1;
        for (const auto& [number_col , col] : sheet_) {
            if (col.count(pos.row) == 1) {
                number_element_in_row++;
            }
            for (const auto& [number_row, row] : sheet_.at(number_col)) {
                if (number_row == extreme_row) {
                    continue;
                }

                if (number_row > new_extreme_row) {
                    new_extreme_row = number_row;
                }
            }
        }
        if (!(number_element_in_row > 1)) {
            result.row = new_extreme_row;
        }
    }

    return  result;
}