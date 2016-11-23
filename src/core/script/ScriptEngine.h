#pragma once
#include <angelscript.h>
#include <string>
#include <vector>
#define g_ScriptEngine ScriptEngine::GetInstance()

class ScriptEngine {
public:
	~ScriptEngine();
	static ScriptEngine& GetInstance();
	void Init();
	void CompileScript(const std::string& scriptname);
	void RunScript(const std::string& scriptname);
	AngelScript::asIScriptEngine* GetEngine() { return m_Engine; }
	void RecompileScript(const std::string& scriptname);
	void RecompileAllScripts();
	void ExecuteString(const std::string& code);
private:
	ScriptEngine();
	std::vector<std::string> m_Scripts;
	AngelScript::asIScriptEngine* m_Engine;
	AngelScript::asIScriptContext* m_Context;
};