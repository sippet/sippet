// Copyright (c) 2015 The Sippet Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "base/at_exit.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/threading/thread.h"
#include "base/files/file_util.h"
#include "base/i18n/icu_util.h"
#include "base/message_loop/message_loop.h"
#include "gin/array_buffer.h"
#include "gin/modules/console.h"
#include "gin/modules/module_registry.h"
#include "gin/modules/module_runner_delegate.h"
#include "gin/public/isolate_holder.h"
#include "gin/try_catch.h"
#include "sippet/phone/v8/phone_js_wrapper.h"

#ifdef V8_USE_EXTERNAL_STARTUP_DATA
#include "gin/v8_initializer.h"
#endif

namespace gin {
namespace {

std::string Load(const base::FilePath& path) {
  std::string source;
  if (!ReadFileToString(path, &source))
    LOG(FATAL) << "Unable to read " << path.LossyDisplayName();
  return source;
}

void Run(base::WeakPtr<Runner> runner, const base::FilePath& path) {
  if (!runner)
    return;
  Runner::Scope scope(runner.get());
  runner->Run(Load(path), path.AsUTF8Unsafe());
}

void ConsoleLoop(Runner* runner, base::MessageLoop* parent_message_loop) {
  std::cerr << "> ";
  std::string line;
  if (std::getline(std::cin, line)
      && !base::StringPiece(line).starts_with("quit()")) {
    parent_message_loop->PostTask(FROM_HERE,
        base::Bind(&Runner::Run, base::Unretained(runner), line, "(shell)"));
    base::MessageLoop::current()->PostTask(FROM_HERE,
        base::Bind(&ConsoleLoop, runner, parent_message_loop));
  } else {
    std::cerr << std::endl;
    parent_message_loop->PostTask(FROM_HERE,
        base::Bind(&base::MessageLoop::Quit,
                   base::Unretained(parent_message_loop)));
  }
}

void OnRunShell(base::WeakPtr<Runner> runner, Arguments args) {
  v8::Isolate *isolate = runner->GetContextHolder()->isolate();

  std::cerr << "Sippet shell (V8 version "
            << v8::V8::GetVersion()
            << ")" << std::endl;

  v8::Handle<v8::Value> sippet;
  v8::Handle<v8::Value> console;
  args.GetNext(&sippet);
  args.GetNext(&console);
  runner->GetContextHolder()->context()->Global()
      ->Set(StringToV8(isolate, "sippet"), sippet);
  runner->GetContextHolder()->context()->Global()
      ->Set(StringToV8(isolate, "console"), console);

  base::Thread console_thread("Console");
  console_thread.Start();
  console_thread.message_loop()->PostTask(FROM_HERE,
      base::Bind(&ConsoleLoop, runner.get(), base::MessageLoop::current()));

  base::MessageLoop::current()->Run();
}

void RunShell(base::WeakPtr<Runner> runner) {
  Runner::Scope scope(runner.get());
  v8::Isolate *isolate = runner->GetContextHolder()->isolate();

  v8::Handle<v8::String> id(StringToV8(isolate, "sippet"));

  v8::Handle<v8::Array> modules(v8::Array::New(isolate));
  modules->Set(0, gin::StringToV8(isolate, "sippet/phone"));
  modules->Set(1, gin::StringToV8(isolate, "console"));

  v8::Handle<v8::FunctionTemplate> function_tmpl(
      CreateFunctionTemplate(isolate, base::Bind(OnRunShell, runner)));

  v8::Handle<v8::Function> function(function_tmpl->GetFunction());

  v8::Handle<v8::Value> args[] = {
    id,
    modules,
    function
  };

  v8::Handle<v8::Value> value(runner->GetContextHolder()->context()->Global()
      ->Get(v8::String::NewFromUtf8(isolate, "define")));
  v8::Handle<v8::Function> define_function(
      v8::Handle<v8::Function>::Cast(value));

  define_function->Call(runner->GetContextHolder()->context()->Global(),
      3, args);
}

std::vector<base::FilePath> GetModuleSearchPaths() {
  std::vector<base::FilePath> module_base(1);
  CHECK(base::GetCurrentDirectory(&module_base[0]));
  return module_base;
}

class GinShellRunnerDelegate : public ModuleRunnerDelegate {
 public:
  GinShellRunnerDelegate() : ModuleRunnerDelegate(GetModuleSearchPaths()) {
    AddBuiltinModule(Console::kModuleName, Console::GetModule);
    AddBuiltinModule(sippet::phone::PhoneJsModule::kName,
        sippet::phone::PhoneJsModule::GetModule);
  }

  void UnhandledException(ShellRunner* runner, TryCatch& try_catch) override {
    ModuleRunnerDelegate::UnhandledException(runner, try_catch);
    LOG(ERROR) << try_catch.GetStackTrace();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(GinShellRunnerDelegate);
};

}  // namespace
}  // namespace gin

int main(int argc, char** argv) {
  base::AtExitManager at_exit;
  base::CommandLine::Init(argc, argv);
  base::i18n::InitializeICU();
#ifdef V8_USE_EXTERNAL_STARTUP_DATA
  gin::V8Initializer::LoadV8Snapshot();
  gin::V8Initializer::LoadV8Natives();
#endif

  base::MessageLoop message_loop;

  gin::IsolateHolder::Initialize(gin::IsolateHolder::kStrictMode,
                                 gin::ArrayBufferAllocator::SharedInstance());
  gin::IsolateHolder instance;


  gin::GinShellRunnerDelegate delegate;
  gin::ShellRunner runner(&delegate, instance.isolate());

  {
    gin::Runner::Scope scope(&runner);
    runner.GetContextHolder()
        ->isolate()
        ->SetCaptureStackTraceForUncaughtExceptions(true);
  }

  base::CommandLine::StringVector args =
      base::CommandLine::ForCurrentProcess()->GetArgs();
  if (args.size() == 0) {
    gin::RunShell(runner.GetWeakPtr());
  } else {
    for (base::CommandLine::StringVector::const_iterator it = args.begin();
      it != args.end(); ++it) {
      base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(
          gin::Run, runner.GetWeakPtr(), base::FilePath(*it)));
    }
    message_loop.RunUntilIdle();
  }
  return 0;
}
