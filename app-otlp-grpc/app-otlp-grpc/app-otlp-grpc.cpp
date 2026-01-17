#include <iostream>
#include <windows.h>
#include "utility/opt_tracer.h"


int main()
{
	InitTracer();


	// トレースの取得
	auto tracer = GetTracer();
	// スパンの作成
	auto span = tracer->StartSpan("sample-span");
	// スパン内での処理
	span->SetAttribute("attribute-key", "attribute-value");
	span->AddEvent("sample-event");

	::Sleep( 1000 ); // 処理のシミュレーション

	// スパンの終了
	span->End();


	std::cin >> std::ws; // ユーザー入力を待機

	CleanupTracer();

	return 0;
}
