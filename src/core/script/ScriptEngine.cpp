#include "ScriptEngine.h"
#include "scriptstdstring/scriptstdstring.h"
#include "scriptbuilder/scriptbuilder.h"
#include <angelscript-integration/angelscript-integration.h>
#include <assert.h>

void MessageCallback(const AngelScript::asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if (msg->type == AngelScript::asMSGTYPE_WARNING)
		type = "WARN";
	else if (msg->type == AngelScript::asMSGTYPE_INFORMATION)
		type = "INFO";
	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

void Print(const std::string& msg) {
	printf("%s", msg.c_str());
}

ScriptEngine::ScriptEngine() {
}

ScriptEngine::~ScriptEngine() {
	m_Context->Release();
	m_Engine->ShutDownAndRelease();
}

ScriptEngine& ScriptEngine::GetInstance() {
	static ScriptEngine m_Instance;
	return m_Instance;
}

void ScriptEngine::Init() {
	m_Engine = AngelScript::asCreateScriptEngine();
	m_Engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, AngelScript::asCALL_CDECL);

	RegisterStdString(m_Engine);

	m_Engine->RegisterGlobalFunction("void print(const string &in)", AngelScript::asFUNCTION(Print), AngelScript::asCALL_CDECL);
	AngelScriptIntegration::init_glm(m_Engine, AngelScriptIntegration::GlmFlags::NO_SWIZZLE);

	m_Context = m_Engine->CreateContext();
}

void ScriptEngine::CompileScript(const std::string& scriptname) {
	AngelScript::CScriptBuilder scriptBuilder;
	int r;
	r = scriptBuilder.StartNewModule(m_Engine, scriptname.c_str()); assert(r >= 0);
	r = scriptBuilder.AddSectionFromFile(scriptname.c_str()); assert(r >= 0);
	r = scriptBuilder.BuildModule(); assert(r >= 0);
	m_Scripts.push_back(scriptname);
	printf("Compiled script :%s\n", scriptname.c_str());
}

void ScriptEngine::RunScript(const std::string& scriptname) {
	AngelScript::asIScriptModule* module = m_Engine->GetModule(scriptname.c_str());
	AngelScript::asIScriptFunction* func = module->GetFunctionByDecl("void main()");
	m_Context->Prepare(func);
	m_Context->Execute();
}

void ScriptEngine::RecompileScript(const std::string& scriptname) {
	m_Engine->DiscardModule(scriptname.c_str());
	AngelScript::CScriptBuilder scriptBuilder;
	int r;
	r = scriptBuilder.StartNewModule(m_Engine, scriptname.c_str()); assert(r >= 0);
	r = scriptBuilder.AddSectionFromFile(scriptname.c_str()); assert(r >= 0);
	r = scriptBuilder.BuildModule(); assert(r >= 0);
}

void ScriptEngine::RecompileAllScripts() {
	for (auto& script : m_Scripts) {
		RecompileScript(script);
	}
}