#pragma once
namespace math
{
    template<typename T>
    class Number
    {
    private:
        T value;

    public:
        // Constructor that takes any tokenType U and performs static_cast
        Number(int8_t _value) : value(static_cast<T>(_value)) {}
        Number(int16_t _value) : value(static_cast<T>(_value)) {}
        Number(uint16_t _value) : value(static_cast<T>(_value)) {}
        Number(int32_t _value) : value(static_cast<T>(_value)) {}
        Number(uint32_t _value) : value(static_cast<T>(_value)) {}
        Number(int64_t _value) : value(static_cast<T>(_value)) {}
        Number(uint64_t _value) : value(static_cast<T>(_value)) {}
        Number(float _value) : value(static_cast<T>(_value)) {}
        Number(double _value) : value(static_cast<T>(_value)) {}

		T& operator*()
        {
            return value;
        }
        

         operator int8_t() const { return static_cast<int8_t>(value); }
         operator int16_t() const { return static_cast<int16_t>(value); }
         operator uint16_t() const { return static_cast<uint16_t>(value); }
         operator int32_t() const { return static_cast<int32_t>(value); }
         operator uint32_t() const { return static_cast<uint32_t>(value); }
         operator int64_t() const { return static_cast<int64_t>(value); }
         operator uint64_t() const { return static_cast<uint64_t>(value); }
         operator float() const { return static_cast<float>(value); }
         operator double() const { return static_cast<double>(value); }
    };



}