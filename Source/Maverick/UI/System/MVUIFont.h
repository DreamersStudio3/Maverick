#pragma once

class UTextBlock;

namespace MVUIFont
{
	/** 기본 서체와 크기를 유지하고, 없는 글자는 엔진 기본 글꼴에서 찾는다. */
	void AddDefaultFallback(UTextBlock& TextBlock);
}
