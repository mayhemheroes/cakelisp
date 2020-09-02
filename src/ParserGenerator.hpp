#pragma once

#include "EvaluatorEnums.hpp"

#include <string>
#include <vector>
// TODO: Replace with fast hash table
#include <unordered_map>

struct NameStyleSettings;
struct Token;

// Rather than needing to allocate and edit a buffer eventually equal to the size of the final
// output, store output operations instead. This also facilitates source <-> generated mapping data
struct StringOutput
{
	std::string output;
	StringOutputModifierFlags modifiers;

	// Used to correlate Cakelisp code with generated output code
	const Token* startToken;
	const Token* endToken;
};

struct FunctionArgumentMetadata
{
	std::string name;

	const Token* typeStartToken;
	// Unnecessary because we can just keep track of our parens, but included for convenience
	const Token* typeEndToken;
};

struct FunctionMetadata
{
	// The Cakelisp name, NOT the converted C name
	std::string name;

	// From the opening paren of e.g. "(defun" to the final ")" closing the body
	const Token* startDefinitionToken;
	// Unnecessary because we can just keep track of our parens, but included for convenience
	const Token* endDefinitionToken;

	std::vector<FunctionArgumentMetadata> arguments;
};

const char* importLanguageToString(ImportLanguage type);

struct ImportMetadata
{
	std::string importName;
	ImportLanguage language;
	const Token* triggerToken;
};

struct GeneratorOutput
{
	std::vector<StringOutput> source;
	std::vector<StringOutput> header;

	std::vector<FunctionMetadata> functions;
	std::vector<ImportMetadata> imports;
};

struct EvaluatorContext
{
	EvaluatorScope scope;
};

struct EvaluatorEnvironment;

// Generators output C/C++ code
typedef bool (*GeneratorFunc)(EvaluatorEnvironment& environment, const EvaluatorContext& context,
                              const std::vector<Token>& tokens, int startTokenIndex,
                              GeneratorOutput& output);

// Macros output tokens only
// Note that this is for the macro invocation/signature; defining macros is actually done via a
// generator, because macros themselves are implemented in Cakelisp/C++
typedef bool (*MacroFunc)(EvaluatorEnvironment& environment, const EvaluatorContext& context,
                          const std::vector<Token>& tokens, int startTokenIndex,
                          std::vector<Token>& output);

// TODO: Replace with fast hash table implementation
typedef std::unordered_map<std::string, MacroFunc> MacroTable;
typedef std::unordered_map<std::string, GeneratorFunc> GeneratorTable;
typedef MacroTable::iterator MacroIterator;
typedef GeneratorTable::iterator GeneratorIterator;

// Unlike context, which can't be changed, environment can be changed.
// Use care when modifying the environment. Only add things once you know things have succeeded.
// Keep in mind that calling functions which can change the environment may invalidate your pointers
// if things resize.
struct EvaluatorEnvironment
{
	MacroTable macros;
	GeneratorTable generators;

	// We need to keep the tokens macros create around so they can be referenced by StringOperations
	// Token vectors must not be changed after they are created or pointers to Tokens will become
	// invalid. The const here is to protect from that. You can change the token contents, however
	std::vector<const std::vector<Token>*> macroExpansions;

	// Will NOT clean up macroExpansions! Use environmentDestroyMacroExpansionsInvalidateTokens()
	~EvaluatorEnvironment();
};

// Make sure you're ready to do this! (see macroExpansions comment)
// Essentially, this means don't call this function unless you will NOT follow any Token pointers in
// GeneratorOutput, StringOutput, etc., because any of them could be pointers to macro-created
// tokens. Essentially, call this as late as possible
void environmentDestroyMacroExpansionsInvalidateTokens(EvaluatorEnvironment& environment);

int EvaluateGenerate_Recursive(EvaluatorEnvironment& environment, const EvaluatorContext& context,
                               const std::vector<Token>& tokens, int startTokenIndex,
                               GeneratorOutput& output);

// Delimiter template will be inserted between the outputs
int EvaluateGenerateAll_Recursive(EvaluatorEnvironment& environment,
                                  const EvaluatorContext& context, const std::vector<Token>& tokens,
                                  int startTokenIndex, const StringOutput& delimiterTemplate,
                                  GeneratorOutput& output);
