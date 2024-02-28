#pragma once
namespace blueprint
{

	class Initializable
	{
	public:
		virtual void Init() = 0;
		virtual ~Initializable() {}
	};



}