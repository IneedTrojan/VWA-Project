#pragma once
#include <cstdint>
#include <glad/glad.h>

#include "BufferObject.h"
#include "Enum/DataType.h"
#include "utility/EnumLinker.h"


namespace graphics
{
	enum VertexAttributePointers
	{
		Position,
		Normal,
		Tangent,
		UV,
	};


	class VertexArrayObject
	{
		GLuint ID = 0;
	public:

		
		VertexArrayObject() = default;

		GLuint GetID()const { return ID; }

		static std::unique_ptr<VertexArrayObject> Create()
		{
			VertexArrayObject vertexArrayObj;
			glCreateVertexArrays( 1, &vertexArrayObj.ID);
			return std::make_unique<VertexArrayObject>(std::move(vertexArrayObj));
		}
		static VertexArrayObject CreateStack()
		{
			VertexArrayObject vertexArrayObj;
			glCreateVertexArrays(1, &vertexArrayObj.ID);
			return vertexArrayObj;
		}
		VertexArrayObject(VertexArrayObject&& other)noexcept
			:ID(std::exchange(other.ID, 0))
		{

		}
		VertexArrayObject& operator=(VertexArrayObject&& other)noexcept
		{
			if (&other != this)
			{
				free();
				ID = std::exchange(other.ID, 0);
			}
			return *this;
		}
		VertexArrayObject(const VertexArrayObject& other) = delete;
		VertexArrayObject& operator =(const VertexArrayObject& other) = delete;


		void VertexArrayAttribIFormat(int32_t target, int32_t num_elements, int32_t stride, int32_t relativeOffset, DataType dataType, const BufferObject* buffer, int32_t bufferBindingIndex, int32_t bufferOffset = 0)const
		{
			const GLuint BufferID = buffer->GetID();
			glVertexArrayAttribBinding(ID, target, bufferBindingIndex);
			glVertexArrayVertexBuffer(ID, bufferBindingIndex, BufferID, bufferOffset, stride);
			glVertexArrayAttribIFormat(ID, target, num_elements, gl_enum_cast(dataType), relativeOffset);
			Activate(target);
		}
		void VertexArrayAttribFormat(int32_t target, int32_t num_elements, int32_t stride, int32_t relativeOffset, DataType dataType, const BufferObject* buffer, int32_t bufferBindingIndex = 0, int32_t bufferOffset = 0, bool normalize = false)const
		{
			glVertexArrayAttribBinding(ID, static_cast<int32_t>(target), bufferBindingIndex);
			glVertexArrayVertexBuffer(ID, bufferBindingIndex, buffer->GetID(), bufferOffset, stride);
			glVertexArrayAttribFormat(ID, static_cast<int32_t>(target), num_elements, gl_enum_cast(dataType), normalize, relativeOffset);
			Activate(target);
		}
		void Activate(uint32_t target)const
		{
			glEnableVertexArrayAttrib(ID, static_cast<int32_t>(target));
		}
		void Deactivate(uint32_t target)const
		{
			glDisableVertexArrayAttrib(ID, static_cast<int32_t>(target));
		}

		void Bind()const
		{
			glBindVertexArray(ID);
		}


		~VertexArrayObject()
		{
			free();
		}


		void free()
		{
			if (ID != 0)
			{
				glDeleteVertexArrays( 1, &ID);
				ID = 0;
			}
		}
	};
	template <typename T>
	concept has_member_x = requires(T t) {
		{ t.x } -> std::convertible_to<float>;
	};
	template <typename T>
	concept has_member_y = requires(T t) {
		{ t.z } -> std::convertible_to<float>;
	};
	template <typename T>
	concept has_member_z = requires(T t) {
		{ t.y } -> std::convertible_to<float>;
	};
	template <typename T>
	concept has_member_w = requires(T t) {
		{ t.w } -> std::convertible_to<float>;
	};
	template <typename T>
	concept has_index_operator = requires(T t) {
		requires has_member_x<decltype(t[0])>;
	};
	
	template <typename T>
	struct underlying_vector_type {
		using type = std::false_type;
	};
	template <typename T>
	struct underlying_matrix_type {
		using type = std::false_type;
	};
	template <typename T>
	struct underlying_type {
		using type = std::false_type;
	};

	template <typename T>
		requires std::is_arithmetic_v<T> && !std::is_class_v<T> 
	struct underlying_type<T> {
		using type = T; 
	};


	template <typename T>
		requires (has_member_x<T>)
	struct underlying_type<T> {
		using type = decltype(std::declval<T>().x);
	};

	template <typename T>
		requires (has_index_operator<T>)
	struct underlying_matrix_type<T> {
		using type = typename underlying_type<decltype(std::declval<T>()[0])>::type;
	};

	template <typename T>
		requires (has_index_operator<T> && !has_member_x<T>)
	struct underlying_vector_type<T> {
		using type = decltype(std::declval<T>()[0]);
	};

	template<typename T>
	using vector_underlying_prim_type = typename underlying_type<T>::type;

	template<typename T>
	using matrix_underlying_prim_type = typename underlying_matrix_type<T>::type;

	template<typename T>
	using matrix_underlying_vector_type = typename underlying_vector_type<T>::type;


	struct LayoutDescriptor
	{

		enum AttributeType {
			Integer,
			NormalizedInteger,
			Float
		};

		int32_t target = 0;
		DataType dataType = DataType::float32_t;
		int32_t relativeOffset = 0;
		int32_t num_elements;
		AttributeType attributeType = Float;

		template<typename T>
		static constexpr int vector_length()
		{
			return has_member_w<T> ? 4 : has_member_z<T> ? 3 : has_member_y<T> ? 2 : 1;
		}
		template<typename T>
			requires (has_index_operator<T> && !has_member_x<T>)
		static constexpr glm::u64vec2 matrix_dim()
		{
			using underlying = matrix_underlying_vector_type<T>;

			constexpr size_t cols = vector_length<underlying>();
			constexpr size_t rows = sizeof(T)/sizeof(underlying);

			return { cols, rows };
		}
		template <typename T>
		static constexpr DataType getDataType() {
			if constexpr (std::is_same_v<T, std::uint8_t>) {
				return DataType::uint8_t;
			}
			else if constexpr (std::is_same_v<T, std::int8_t>) {
				return DataType::int8_t;
			}
			else if constexpr (std::is_same_v<T, std::uint16_t>) {
				return DataType::uint16_t;
			}
			else if constexpr (std::is_same_v<T, std::int16_t>) {
				return DataType::int16_t;
			}
			else if constexpr (std::is_same_v<T, std::uint32_t>) {
				return DataType::uint32_t;
			}
			else if constexpr (std::is_same_v<T, std::int32_t>) {
				return DataType::int32_t;
			}
			else if constexpr (std::is_same_v<T, std::uint64_t>) {
				return DataType::uint64_t;
			}
			else if constexpr (std::is_same_v<T, std::int64_t>) {
				return DataType::int64_t;
			}
			else if constexpr (std::is_same_v<T, double>) {
				return DataType::float64_t;
			}
			else if constexpr (std::is_same_v<T, float>) {
				return DataType::float32_t;
			}
			else if constexpr (std::is_same_v<T, std::uint16_t>) { 
				return DataType::uint16_t;
			}
			else {
				return DataType::NONE;
			}
		}
		template<typename T, typename...args>
			requires (...&&std::is_same_v<DataType, args>)
		static constexpr bool IsDataTypeOrUnderlying(args...type)
		{
			using underlyingType = vector_underlying_prim_type<T>;

			DataType t = LayoutDescriptor::getDataType<underlyingType>();
			return (... || (type == t));
		}

		template<typename T, typename...args>
			requires ((...&&std::is_same_v<DataType, args>))
		static constexpr bool IsDataTypeOrUnderlyingMatrix (args...type)
		{
			using underlyingType = matrix_underlying_prim_type<T>;

			DataType t = LayoutDescriptor::getDataType<underlyingType>();
			return (... || (type == t));
		}

		template<typename Struct, typename MemberType>
		static LayoutDescriptor construct(int32_t target, int32_t offset, bool normalized = false)
		{
			using underlyingType = vector_underlying_prim_type<MemberType>;

			LayoutDescriptor layout;
			layout.target = target;
			layout.dataType = getDataType<underlyingType>();
			layout.relativeOffset = offset;
			layout.num_elements = vector_length<MemberType>();
			layout.attributeType = std::is_floating_point_v<underlyingType> ? Float : normalized ? NormalizedInteger : Integer;

			return layout;
		}

	};

	struct BufferDescriptor
	{
		const BufferObject* bufferObject = nullptr;
		int32_t vboBinding = 0;
		int32_t structSize = 0;
		int32_t bufferOffset = 0;
	};

	struct VertexAttributePointer : LayoutDescriptor, BufferDescriptor
	{
		VertexAttributePointer(const LayoutDescriptor& _layout, const BufferDescriptor& _buffer)
			: LayoutDescriptor(_layout), BufferDescriptor(_buffer)
		{
		}
	};


	class VertexArray
	{

		VertexArrayObject vertexArray;
		std::vector<VertexAttributePointer> vertexAttributePointers;
		int32_t bufferBinding = 0;
	public:

		static std::unique_ptr<VertexArray> Create(VertexArrayObject&& vao = VertexArrayObject::CreateStack())
		{
			return std::unique_ptr<VertexArray>(new VertexArray(std::move(vao)));
		}
		static VertexArray CreateStack(VertexArrayObject&& vao = VertexArrayObject::CreateStack())
		{
			return VertexArray(std::move(vao));
		}

		void VertexAttribPointer(LayoutDescriptor descriptor, const BufferObject& vbo, int32_t structSize, int32_t bufferOffset)
		{
			std::vector<VertexAttributePointer>::const_iterator it = std::ranges::find_if(
				vertexAttributePointers.begin(), vertexAttributePointers.end(),
				[&vbo](const VertexAttributePointer& attribute) {
					return attribute.bufferObject == &vbo;
				});
			const int32_t vboBinding = it == vertexAttributePointers.end() ? bufferBinding++ : it->vboBinding;

			vertexAttributePointers.emplace_back(descriptor, BufferDescriptor(&vbo, vboBinding, structSize, bufferOffset));
		}
		void Finalize()const
		{
			for (const auto& attribute : vertexAttributePointers)
			{
				switch (attribute.attributeType)
				{
				case LayoutDescriptor::Integer:
					vertexArray.VertexArrayAttribIFormat(
						attribute.target,
						attribute.num_elements,
						attribute.structSize,
						attribute.relativeOffset,
						attribute.dataType,
						attribute.bufferObject,
						attribute.vboBinding,
						attribute.bufferOffset
					);
					break;
				case LayoutDescriptor::NormalizedInteger:
					throw std::exception("not implemented yet");
				case LayoutDescriptor::Float:

					vertexArray.VertexArrayAttribFormat(
						attribute.target,
						attribute.num_elements,
						attribute.structSize,
						attribute.relativeOffset,
						attribute.dataType,
						attribute.bufferObject,
						attribute.vboBinding,
						attribute.bufferOffset,
						attribute.attributeType == LayoutDescriptor::Float
					);

					break;
				}

			}
		}
		void Reset()
		{
			for (const auto& attribute : vertexAttributePointers)
			{
				vertexArray.Deactivate(attribute.target);
			}
			vertexAttributePointers.clear();
			bufferBinding = 0;
		}
		void Bind()const
		{
			vertexArray.Bind();
		}

		operator const VertexArrayObject* ()const
		{
			return &vertexArray;
		}
		const VertexArrayObject& Base()const
		{
			return vertexArray;
		}
	private:
		VertexArray(VertexArrayObject&& vao) : vertexArray(std::move(vao))
		{

		}
	};
	struct MultiVertexArray
	{
		std::vector<VertexAttributePointer> vertexAttributePointers;
		mutable std::vector<std::unique_ptr<VertexArray>> vao;
		mutable std::vector<uint32_t> vaoMask;
		uint32_t attributeMask;

		MultiVertexArray()
			: attributeMask(0)
		{

		}

		VertexAttributePointer* getAttributePointer(int32_t target)
		{
			VertexAttributePointer* ptr = nullptr;
			auto it = std::ranges::find_if(vertexAttributePointers.begin(), vertexAttributePointers.end(),
				[target](const VertexAttributePointer& attribute) {
					return attribute.target == target;
				});
			if (it != vertexAttributePointers.end())
			{
				ptr = &*it;
			}
			return ptr;
		}

		void initialize(utility::const_span<VertexAttributePointer> vertexLayout)
		{
			vertexAttributePointers = std::vector(vertexLayout.begin_ptr, vertexLayout.end_ptr);

			for (const auto& vertexAttributePointer : vertexAttributePointers)
			{
				attributeMask |= 1u << vertexAttributePointer.target;
			}
		}

		void ActivateVAO(uint32_t mask)const
		{
			mask = mask & attributeMask;
			const auto match = std::ranges::find(vaoMask.begin(), vaoMask.end(), mask);
			if (match == vaoMask.end())
			{
				initializeVAO(mask).Bind();
			}
			else
			{
				const auto distance = std::distance(vaoMask.begin(), match);
				vao[distance]->Bind();
			}
		}
		void Reset()const
		{
			vao.clear();
			vaoMask.clear();
		}
		MultiVertexArray Clone(const BufferObject* buffer)const
		{
			auto cpy = vertexAttributePointers;
			for (auto& clone : cpy)
			{
				clone.bufferObject = buffer;
			}
			MultiVertexArray temp;
			temp.initialize(cpy);
			return temp;
		}

	private:
		VertexArray& initializeVAO(uint32_t mask)const
		{
			vaoMask.emplace_back(mask);
			const std::unique_ptr<VertexArray>& newVAO = vao.emplace_back(VertexArray::Create());

			for (const auto& vertexAttributePointer : vertexAttributePointers)
			{
				if ((1u << vertexAttributePointer.target & mask) != 0u)
				{
					newVAO->VertexAttribPointer(
						static_cast<LayoutDescriptor>(vertexAttributePointer),
						*vertexAttributePointer.bufferObject,
						vertexAttributePointer.structSize,
						vertexAttributePointer.bufferOffset
					);
				}
			}
			newVAO->Finalize();
			return *newVAO;
		}
	};

}
#define attributePointer(target, Struct, Member) \
    graphics::LayoutDescriptor::construct<Struct, decltype(Struct::Member)>(target, offsetof(Struct, Member), false)

#define normalizedAttributePointer(target, Struct, Member) \
   graphics::LayoutDescriptor::construct<Struct, decltype(Struct::Member)>(target, offsetof(Struct, Member), true)
