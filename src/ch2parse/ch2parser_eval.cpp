/**
* @file ch2parser_eval.cpp
* @author lakor64
* @date 20/05/2024
* @brief evalutation code
*/
#include "ch2parser.hpp"
#include <exprtk.hpp>

void CH2Parser::EvalDefine(Define* def)
{
	exprtk::expression<double> expression;
	exprtk::parser<double> parser;

	if (!parser.compile(def->GetValue(), expression))
	{
		m_lasterr = CH2ErrorCodes::EvalError;
		return;
	}

	auto res = expression.value();

	std::stringstream stream;

	if (def->GetDefineType() == DefineType::Float)
		stream << res;
	else
		stream << std::hex << uint32_t(res);

	def->m_defType = DefineType::Hexadecimal;
	def->m_value = stream.str();
}
