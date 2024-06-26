#pragma once

#include <string>
#include <map>
#include <fixedpoint.h>
#include <mutex>

#include "serial.h"

namespace Globals {

	enum GlobalVariableTypes {
		I64,
		U64,
		F64,
		I32,
		U32,
		FP32,
		F32,
		I16,
		U16,
		I8,
		U8,
		BOOL
	};

	union GlobalVariableValue {
		int64_t i64;
		uint64_t u64;
		double f64;
		int32_t i32;
		uint32_t u32;
		fp32_t fp32;
		float f32;
		int16_t i16;
		uint16_t u16;
		int8_t i8;
		uint8_t u8;
		bool b;
	};

	//struct GlobalVariable {
	//	uint16_t id = 0;
	//	uint8_t type;
	//	GlobalVariableValue value;
	//};

	struct GlobalVariable {
		std::string name;
		uint8_t type;
		GlobalVariableValue value;
	};

	class Globals {
	private:
		int valueCount = 0;

		Serial::Port* port;

		void setVarFromPointer(GlobalVariable& var, void* pointer);
		void setPointerFromVar(GlobalVariable& var, void* pointer);

	public:
		Globals(Serial::Port& port);

		std::map<uint16_t, GlobalVariable> variables;
		std::mutex mutex;

		size_t getVariableSize(const GlobalVariable& var);

		void close();

		void pollAll();
		void pollList(std::vector<uint16_t> list);
		void setList(std::vector<uint16_t> list);
		void loadList(std::vector<uint16_t> list);
		void saveList(std::vector<uint16_t> list);
	};

}