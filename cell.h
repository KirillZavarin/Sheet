#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <functional>
#include <optional>
#include <unordered_set>


class Sheet;

class Cell : public CellInterface {
    
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text) override;
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl {
    public:
        virtual Value GetValue(Sheet* sheet_) const = 0;
        virtual std::string GetText() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        Value GetValue(Sheet*  /*sheet*/) const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        TextImpl(std::string text);

        Value GetValue(Sheet* /*sheet*/) const override;
        std::string GetText() const override;

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
        friend Cell;
    public:
        FormulaImpl(std::string text);
        Value GetValue(Sheet* sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const;
    private:
        std::unique_ptr<FormulaInterface> formula_;
        mutable std::optional<double> cache_ = std::nullopt;
    };

    void InvalidatorCache();

    void SetEdges(const std::vector<Position>& list);

    void ClearEdges();

    bool CheckCycles(const std::vector<Position>& list) const;

    bool SearchInDepth(std::unordered_set<const CellInterface*> visited_cells) const;

    void SetEmptyImpl();

    void SetTextImpl(std::string text);

    void SetFormulaImpl(std::string text);

    std::unordered_set<CellInterface*> incoming_edges_;
    std::unordered_set<CellInterface*> outgoing_edges_;

    std::unique_ptr<Impl> impl_;
    Sheet* sheet_;

    // Добавьте поля и методы для связи с таблицей, проверки циклических 
    // зависимостей, графа зависимостей и т. д.
    friend FormulaImpl;
};