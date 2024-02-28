#pragma once
#include <string>

#include "gl/Texture/Texture.h"
#include <type_traits>
#include <cstddef>
#include "math/PointerMath.h"





namespace renderer
{
	class param;
	class param {
	public:
		void* self;
		bool isPointer;

		param(void* obj, bool isptr) :self(obj), isPointer(isptr)
		{

		}

		void* valuePtr()
		{
			void* pointer = nullptr;
			if (self != nullptr)
			{
				if (isPointer) {
					// Self is a pointer to a pointer
					pointer = self;
				}
				else {
					// Self is a pointer to an object
					pointer = static_cast<void*>(&self);
				}
			}
			return pointer;
		}
		const void* valuePtr()const
		{
			const void* pointer = nullptr;
			if (self != nullptr)
			{
				if (isPointer) {
					// Self is a pointer to a pointer
					pointer = *static_cast<const void**>(self);
				}
				else {
					// Self is a pointer to an object
					pointer = reinterpret_cast<const void*>(&self);
				}
			}
			return pointer;
		}

		bool operator==(const param& other) const {
			return valuePtr() == other.valuePtr();
		}
	};







	template<typename T>
	class TParam
	{
	public: T self;

		  template<typename... Args>
		  TParam(Args&&... args) :
			  self(std::forward<Args>(args)...)
		  {
		  }


		  auto& operator*() {
			  if constexpr (std::is_pointer<T>::value) {
				  return *self;
			  }
			  else {
				  return self;
			  }
		  }
		  auto operator->() {
			  if constexpr (std::is_pointer<T>::value) {
				  return self;
			  }
			  else {
				  return &self;
			  }
		  }



		  param removeTemplate()
		  {
			  if constexpr (std::is_pointer<T>::value) {
				  return param(&self, true);
			  }
			  else {
				  return param(&self, false);
			  }
		  }
	};
	

#define CONCATENATE_INTERNAL(param1, param2) param1##param2
#define CONCATENATE(param1, param2) CONCATENATE_INTERNAL(param1, param2)

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)



#define MAKE_STRING(param1, param2) STRINGIFY(CONCATENATE(param1, param2))


#define input(type, name) \
    Input<type> name; \
    public: inputField IN##name = inputField(inputs, MAKE_STRING(IN, name), param(name##.removeTemplate()), this)


#define output(type, name) \
    Output<type> name; \
    public: inputField OUT##name = inputField(outputs, MAKE_STRING(OUT, name), param(name##.removeTemplate()), this)




	class GraphNode
	{
	protected:
		std::map <std::string, param> inputs;
		std::map <std::string, param> outputs;
	public:

		GraphNode()
		{
			
		}

		void setInput(const std::string& _name, param* other)
		{
			auto it = inputs.find(_name);
			if(it != inputs.end())
			{
				memcpy(it->second.valuePtr(), other->valuePtr(), 8);
			}
		}
		param* getOutput(const std::string& _name)
		{
			auto it = outputs.find(_name);
			param* p = nullptr;
			if (it != outputs.end())
			{
				p = &it->second;
			}
			return p;
		}


		virtual void Initialize() = 0;
		virtual void Execute() = 0;
		virtual ~GraphNode(){}
	};
	
	template<typename T>
	using Input = TParam<T>;
	template<typename T>
	using Output = TParam<T>;

	struct inputField {
		std::string _parameterName;
		GraphNode* self;
		param parameter;

		inputField(std::map<std::string, param>& map, std::string name, param _parameter, GraphNode* _self)
			: _parameterName(name), self(_self), parameter(_parameter) {
			map.emplace(std::move(name), _parameter);

		}



	};


#if 0


	class Branch
	{
	public:
		Branch(const inputField& _out,const inputField& _in) :
			out(_out), in(_in)
		{

		}
		inputField out;
		inputField in;
	};

	template<typename T>
	struct ptr {
		T* self;

		ptr(T* p) : self(p) {}

		T* operator->() {
			return self;
		}

		// Dereference operator for non-void types
		T& operator*() {
			static_assert(!std::is_same<T, void>::value, "Cannot dereference void pointer");
			return *self;
		}

		// Equality operator for ptr-to-void comparison
		template<typename U>
		bool operator==(const ptr<U>& other) const {
			return self == other.self;
		}

		// Equality operator for direct void* comparison
		bool operator==(const void* other) const {
			return static_cast<const void*>(self) == other;
		}

		// Inequality operator for ptr-to-void comparison
		template<typename U>
		bool operator!=(const ptr<U>& other) const {
			return !(*this == other);
		}

		// Inequality operator for direct void* comparison
		bool operator!=(const void* other) const {
			return !(*this == other);
		}
	};


	inline std::vector<GraphNode*> sortRenderPasses(const std::vector<Branch>& branches)
	{
		using Node = GraphNode*;

		std::map<Node, std::vector<Branch>> flippedGraph;

		for (const auto& branch : branches)
		{
			auto it = flippedGraph.find(branch.in.self);
			if (it == flippedGraph.end_index())
			{
				it = flippedGraph.emplace(branch.in.self, std::vector<Branch>{}).first;
			}
			it->second.emplace_back(branch);
		}

		std::vector<Node> sorted;
		std::set<Node> visited;

		std::function<void(Node)> visitNode;
		visitNode = [&](Node node) {
			if (!visited.contains(node)) {
				visited.insert(node);

				const std::vector<Branch>& parameters = flippedGraph[node];
				for (const auto& param : parameters) {
					visitNode(param.out.self);

					inputField in = param.in;
					inputField out = param.out;
					in.self->setInput(in._parameterName, &out.parameter);
				}

				sorted.push_back(node);
				node->Initialize();
			}
		};
		for (auto& node : flippedGraph)
		{
			visitNode(node.first);
		}
		return sorted;
	}




	class SubGraph : public GraphNode
	{
		std::vector<Branch> branches;

		std::vector<GraphNode*> passes;
	public:
		SubGraph()
		{
		}

		void add_branch(inputField& a, inputField& b)
		{
			branches.emplace_back(a, b);
		}

		virtual void Initialize()override
		{
			passes = sortRenderPasses(branches);
		}


		virtual void Execute() override
		{
			for (const auto pass : passes)
			{
				pass->Execute();
			}
		}
	};
	

#endif

}
