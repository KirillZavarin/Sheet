#include "cell.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <optional>

using namespace std::literals;
// Реализуйте следующие методы
Cell::Cell(Sheet& sheet) : impl_(new EmptyImpl) {
	sheet_ = &sheet;
}

Cell::~Cell(){}

void Cell::SetEmptyImpl() {	
	if (impl_.get() && dynamic_cast<FormulaImpl*>(impl_.get()) != nullptr) {
		InvalidatorCache();
	}
	impl_ = std::unique_ptr<Impl>(new EmptyImpl);
	outgoing_edges_.clear();
}

void Cell::SetTextImpl(std::string text) {
	if (impl_.get() && dynamic_cast<FormulaImpl*>(impl_.get()) != nullptr) {
		InvalidatorCache();
	}
	impl_ = std::unique_ptr<Impl>(new TextImpl(text));
	outgoing_edges_.clear();
}

void Cell::SetFormulaImpl(std::string text) {
	auto old_impl = std::move(impl_);

	try {
		impl_ = std::unique_ptr<Impl>(new FormulaImpl(text));
	}
	catch (...) {
		throw FormulaException("Pos in formula is't valid");
	}

	for (const auto pos : dynamic_cast<FormulaImpl*>(impl_.get())->GetReferencedCells()) {
		if (!pos.IsValid()) {
			throw FormulaException("Pos in formula is't valid");
		}
	}

	if (!CheckCycles(dynamic_cast<FormulaImpl*>(impl_.get())->GetReferencedCells())) {
		impl_.swap(old_impl);
		throw CircularDependencyException("Cyclic dependence");
	}


	ClearEdges();
	SetEdges(dynamic_cast<FormulaImpl*>(impl_.get())->GetReferencedCells());


	if (impl_.get() && dynamic_cast<FormulaImpl*>(impl_.get()) != nullptr) {
		InvalidatorCache();
	}
}

void Cell::Set(std::string text) {
	if (text == ""s) {
		SetEmptyImpl();
		return;
	}

	if (text[0] == '=' && text.size() != 1) {
		SetFormulaImpl(std::string(text.begin() + 1, text.end()));
	}
	else {
		SetTextImpl(text);
	}
}

void Cell::Clear() { impl_ = std::unique_ptr<Impl>(new EmptyImpl); }

Cell::Value Cell::GetValue() const {
	return impl_->GetValue(sheet_);
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

Cell::Value Cell::EmptyImpl::GetValue(Sheet* /*sheet*/) const {
	return ""s;
}

std::string Cell::EmptyImpl::GetText() const {
	return ""s;
}

Cell::TextImpl::TextImpl(std::string text) : text_(text) {
}

Cell::Value Cell::TextImpl::GetValue(Sheet* /*sheet*/) const {
	if (text_[0] == '\'') {
		return std::string(text_.begin() + 1, text_.end());
	}

	if (std::find_if(text_.begin(), text_.end(), [](unsigned char c) { return !std::isdigit(c); }) == text_.end()) {
		std::stringstream ss;
		ss << text_;
		double result;
		ss >> result;
		return result;
	}

	return text_;
}

std::string Cell::TextImpl::GetText() const {
	return text_;
}

Cell::FormulaImpl::FormulaImpl(std::string text) : formula_(std::move(ParseFormula(text))) {}

Cell::Value Cell::FormulaImpl::GetValue(Sheet* sheet) const {
	if (cache_.has_value()) {
		return cache_.value();
	}
	auto result = formula_->Evaluate(*sheet);
	if (std::holds_alternative<double>(result)) {
		cache_ = std::get<double>(result);
		return std::get<double>(result);
	}
	return std::get<FormulaError>(result);
}

std::string Cell::FormulaImpl::GetText() const {
	return "="s + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return formula_->GetReferencedCells();
}

void Cell::InvalidatorCache() {
	for (CellInterface* cell : outgoing_edges_) {
		FormulaImpl* formula_ptr = dynamic_cast<Cell::FormulaImpl*>(dynamic_cast<Cell*>(cell)->impl_.get());
		if (formula_ptr && !formula_ptr->cache_) {
			dynamic_cast<Cell*>(cell)->InvalidatorCache();
			formula_ptr->cache_.reset();
		}
	}
}

void Cell::ClearEdges() {
	for (auto cell : outgoing_edges_) {
		dynamic_cast<Cell*>(cell)->incoming_edges_.erase(this);
	}
	outgoing_edges_.clear();
}

void Cell::SetEdges(const std::vector<Position>& list) {
	for (Position cell : list) {
		auto cell_ptr = dynamic_cast<Cell*>(sheet_->GetCellPtr(cell));
		outgoing_edges_.insert(cell_ptr);
		cell_ptr->incoming_edges_.insert(this);
	}
}

bool Cell::CheckCycles(const std::vector<Position>& list) const {
	std::unordered_set<const CellInterface*> visited_cells;
	visited_cells.insert(this);
	for (Position cell : list) {
		if (!dynamic_cast<Cell*>(sheet_->GetCellPtr(cell))->SearchInDepth(visited_cells)) {
			return false;
		}
	}
	return true;
}

bool Cell::SearchInDepth(std::unordered_set<const CellInterface*> visited_cells) const {
	if (!visited_cells.insert(this).second) {
		return false;
	}
	for (const CellInterface* cell : outgoing_edges_) {
		if (!dynamic_cast<const Cell*>(cell)->SearchInDepth(visited_cells)) {
			return false;
		}
	}

	return true;
}

std::vector<Position> Cell::GetReferencedCells() const {
	if (dynamic_cast<FormulaImpl*>(impl_.get())) {
		return dynamic_cast<FormulaImpl*>(impl_.get())->GetReferencedCells();
	}
	return {};
}

bool Cell::IsReferenced() const {
	return !incoming_edges_.empty();
}