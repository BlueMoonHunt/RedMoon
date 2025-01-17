#pragma once
#include <vector>
#include <utility>
#include <print>
#include "rmParallel.h"
#include <algorithm>
#include <random>

namespace rm {

    enum class MatType {
        Zero = 0,
        Random,
        Identity
    };

    template<class T>
    class _Matrix {
    public:
        ~_Matrix() = default;
        _Matrix(size_t rows, size_t columns, MatType type = MatType::Zero)
            : m_Rows(rows), m_Columns(columns), m_Data(rows* columns) {
            switch (type) {
            case MatType::Zero:
                fill(static_cast<T>(0));
                break;
            case MatType::Random: {
                std::random_device rd;
                std::mt19937 gen(rd());
                auto dist = (std::is_unsigned_v<T>) ? std::uniform_real_distribution<T>(0, 1) : std::uniform_real_distribution<T>(-1, 1);
                std::generate(m_Data.begin(), m_Data.end(), [&]() { return dist(gen); });
                break;
            }
            case MatType::Identity:
                std::fill(m_Data.begin(), m_Data.end(), static_cast<T>(0));
                for (size_t i = 0; i < std::min(m_Rows, m_Columns); ++i) {
                    m_Data[i * m_Columns + i] = static_cast<T>(1);
                }
                break;
            }
        }

        _Matrix(const std::initializer_list<T>& list)
            : m_Rows(1), m_Columns(list.size()), m_Data(list) {
        }

        _Matrix(const _Matrix& other)
            : m_Rows(other.m_Rows), m_Columns(other.m_Columns), m_Data(other.m_Data) {
        }

        _Matrix(_Matrix&& other) noexcept
            : m_Rows(std::exchange(other.m_Rows, 0)), m_Columns(std::exchange(other.m_Columns, 0)),
            m_Data(std::move(other.m_Data)) {
        }

        _Matrix& operator=(const _Matrix& other) {
            if (this != &other) {
                m_Rows = other.m_Rows;
                m_Columns = other.m_Columns;
                m_Data = other.m_Data;
            }
            return *this;
        }

        _Matrix& operator=(_Matrix&& other) noexcept {
            if (this != &other) {
                m_Rows = std::exchange(other.m_Rows, 0);
                m_Columns = std::exchange(other.m_Columns, 0);
                m_Data = std::move(other.m_Data);
            }
            return *this;
        }

        T& operator()(size_t row, size_t col) {
            if (row >= m_Rows || col >= m_Columns) {
                throw std::out_of_range("_Matrix indices out of bounds.");
            }
            return m_Data[row * m_Columns + col];
        }

        const T& operator()(size_t row, size_t col) const {
            if (row >= m_Rows || col >= m_Columns) {
                throw std::out_of_range("_Matrix indices out of bounds.");
            }
            return m_Data[row * m_Columns + col];
        }

        size_t getRows() const { return m_Rows; }
        size_t getColumns() const { return m_Columns; }
        
        void setRows(const size_t rows) {
            (rows > m_Data.size() || rows * (m_Data.size() / rows) != m_Data.size()) ? throw std::runtime_error("Invalid Number of Rows.") : 
            m_Rows = rows;
            m_Columns = m_Data.size() / m_Rows;
        }
        
        void setColumns(const size_t columns) {
            (columns > m_Data.size() || columns * (m_Data.size() / columns) != m_Data.size()) ? throw std::runtime_error("Invalid Number of Columns.") : 
            m_Columns = columns;
            m_Rows = m_Data.size() / m_Columns;
        }

        void fill(const T& number) {
            std::fill(m_Data.begin(), m_Data.end(), number);
        }

        void print(size_t maxRows = 10, size_t maxColumns = 10) const {
            size_t printRows = std::min(m_Rows, maxRows);
            size_t printCols = std::min(m_Columns, maxColumns);

            // Calculate the maximum width of each element for formatting
            size_t maxWidth = 0;
            for (size_t i = 0; i < printRows; ++i) {
                for (size_t j = 0; j < printCols; ++j) {
                    maxWidth = std::max(maxWidth, std::to_string(m_Data[i * m_Columns + j]).length());
                }
            }

            std::println("Matrix ({}, {}):", m_Rows, m_Columns);
            for (size_t i = 0; i < printRows; ++i) {
                for (size_t j = 0; j < printCols; ++j) {
                    std::print("{:>{}} ", m_Data[i * m_Columns + j], maxWidth);
                }
                std::println("");
            }

            // Add ellipsis if the _Matrix is truncated
            if (m_Rows > maxRows || m_Columns > maxColumns) {
                std::println("... (_Matrix truncated)");
            }
        }

        _Matrix resize(size_t newRows) const {
            _Matrix<T> resized(*this);
            resized.setRows(newRows);
            return resized;
        }

        _Matrix<T> transpose() const {
            _Matrix<T> result(m_Columns, m_Rows);
            parExecution(m_Rows, [&result, this](size_t start_row, size_t end_row) {
                for (size_t i = start_row; i < end_row; ++i) {
                    for (size_t j = 0; j < m_Columns; ++j) {
                        result(j, i) = (*this)(i, j);
                    }
                }
            });
            return result;
        }

        _Matrix<T>& operator+=(const _Matrix<T>& other) {
            if (!hasSameDimensions(other)) {
                throw std::invalid_argument("_Matrix dimensions must match for addition.");
            }
            parExecution(m_Data.size(), [this, &other](size_t start, size_t end) {
                for (size_t i = start; i < end; ++i) {
                    m_Data[i] += other.m_Data[i];
                }
                });
            return *this;
        }

        _Matrix<T>& operator-=(const _Matrix<T>& other) {
            if (!hasSameDimensions(other)) {
                throw std::invalid_argument("_Matrix dimensions must match for subtraction.");
            }
            parExecution(m_Data.size(), [this, &other](size_t start, size_t end) {
                for (size_t i = start; i < end; ++i) {
                    m_Data[i] -= other.m_Data[i];
                }
                });
            return *this;
        }

        _Matrix<T>& operator*=(const T scalar) {
            parExecution(m_Data.size(), [this, scalar](size_t start, size_t end) {
                for (size_t i = start; i < end; ++i) {
                    m_Data[i] *= scalar;
                }
                });
            return *this;
        }

        _Matrix<T>& operator/=(const T scalar) {
            if (scalar == 0) {
                throw std::runtime_error("Division by zero");
            }
            parExecution(m_Data.size(), [this, scalar](size_t start, size_t end) {
                for (size_t i = start; i < end; ++i) {
                    m_Data[i] /= scalar;
                }
                });
            return *this;
        }

        _Matrix<T> operator+(const _Matrix<T>& other) const { _Matrix<T> result = *this; result += other; return result; }
        _Matrix<T> operator-(const _Matrix<T>& other) const { _Matrix<T> result = *this; result -= other; return result; }
        _Matrix<T> operator/(const T scalar) const { _Matrix<T> result = *this; result /= scalar; return result; }
        _Matrix<T> operator*(const T scalar) const { _Matrix<T> result = *this; result *= scalar; return result; }

        _Matrix<T> operator*(const _Matrix<T>& other) const {
            if (m_Columns != other.m_Rows) {
                throw std::invalid_argument("Incompatible _Matrix dimensions for multiplication.");
            }

            _Matrix<T> result(m_Rows, other.m_Columns);
            parExecution(m_Rows, [&result, this, &other](size_t start_row, size_t end_row) {
                for (size_t i = start_row; i < end_row; ++i) {
                    for (size_t j = 0; j < other.m_Columns; ++j) {
                        T sum = 0;
                        for (size_t k = 0; k < m_Columns; ++k) {
                            sum += (*this)(i, k) * other(k, j);
                        }
                        result(i, j) = sum;
                    }
                }
                });
            return result;
        }

    private:
        bool hasSameDimensions(const _Matrix<T>& other) const {
            return m_Rows == other.m_Rows && m_Columns == other.m_Columns;
        }

    private:
        size_t m_Rows;
        size_t m_Columns;
        std::vector<T> m_Data;
    };


    using mat_t = _Matrix<float>;
    using umat8_t = _Matrix<uint8_t>;
    using umat16_t = _Matrix<uint16_t>;
    using umat32_t = _Matrix<uint32_t>;
    using umat64_t = _Matrix<uint64_t>;

} // namespace rm