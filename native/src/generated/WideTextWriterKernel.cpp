#include <Game/Screen/CustomTagProcessor.hpp>
#include <nw4r/ut/TextWriterBase.h>
#include <nw4r/lyt/textBox.h>
#include <algorithm>
namespace nw4r::ut {
        template <>
        f32 TextWriterBase< wchar_t >::PrintImpl(StreamType str, int length) {
            NW4R_POINTER_ASSERT_AT(308, this);
            NW4R_POINTER_ASSERT_AT(309, str);
            NW4R_POINTER_ASSERT_AT(310, GetFont());
            NW4R_MIN_ASSERT_AT(311, length, 0);
            f32 xOrigin = GetCursorX();
            f32 yOrigin = GetCursorY();
            const bool bUseLimit = (mWidthLimit < nw4r::math::F_MAX);
            const f32 orgCursorX = xOrigin;
            const f32 orgCursorY = yOrigin;
            f32 xCursorAdj = 0.0f;
            f32 yCursorAdj = 0.0f;
            f32 textWidth = 0;
            bool bCharSpace = false;
            StreamType prevStreamPos = str;
            StreamType prevNewLinePos = str;

            {
                textWidth = AdjustCursor(&xOrigin, &yOrigin, str, length);
                xCursorAdj = orgCursorX - GetCursorX();
                yCursorAdj = orgCursorY - GetCursorY();
            }

            PrintContext< CharType > context = {this, str, xOrigin, yOrigin, 0};
            CharStrmReader reader = GetFont()->GetCharStrmReader();
            reader.Set(str);

            CustomTagProcessor* processor = nullptr;
            const u8 alpha = GetAlpha();
            if (&mDefaultTagProcessor != mTagProcessor) {
                processor = static_cast< CustomTagProcessor* >(mTagProcessor->NativeCustomProcessor());
                if(processor) processor->reset(str);
            }

            for (CharCode code = reader.Next(); reinterpret_cast< StreamType >(reader.GetCurrentPos()) - str <= length;) {
                if (code < ' ')

                {
                    typename TagProcessor::Operation operation;
                    context.str = reinterpret_cast< StreamType >(reader.GetCurrentPos());
                    context.flags = 0;
                    context.flags |= bCharSpace ? 0 : CONTEXT_NO_CHAR_SPACE;

                    if (bUseLimit && (code != '\n') && (prevStreamPos != prevNewLinePos)) {
                        PrintContext< CharType > context2 = context;
                        TextWriterBase< CharType > myCopy = *this;
                        Rect rect;

                        context2.writer = &myCopy;
                        operation = mTagProcessor->CalcRect(&rect, code, &context2);

                        if ((rect.GetWidth() > 0.0f) && (myCopy.GetCursorX() - context.xOrigin > mWidthLimit)) {
                            code = '\n';
                            reader.Set(prevStreamPos);
                            continue;
                        }
                    }

                    operation = mTagProcessor->Process(code, &context);

                    if (operation == TagProcessor::OPERATION_NEXT_LINE) {
                        NW4R_POINTER_ASSERT_AT(378, context.str);

                        if (IsDrawFlagSet(HORIZONTAL_ALIGN_MASK, HORIZONTAL_ALIGN_CENTER)) {
                            const int remain = length - (context.str - str);
                            const f32 width = CalcLineWidth(context.str, remain);
                            const f32 offset = (textWidth - width) / 2;
                            SetCursorX(context.xOrigin + offset);
                        } else if (IsDrawFlagSet(HORIZONTAL_ALIGN_MASK, HORIZONTAL_ALIGN_RIGHT)) {
                            const int remain = length - (context.str - str);
                            const f32 width = CalcLineWidth(context.str, remain);
                            const f32 offset = textWidth - width;
                            SetCursorX(context.xOrigin + offset);
                        } else {
                            const f32 width = GetCursorX() - context.xOrigin;
                            textWidth = Max(textWidth, width);

                            SetCursorX(context.xOrigin);
                        }

                        if (bUseLimit) {
                            prevNewLinePos = reinterpret_cast< StreamType >(reader.GetCurrentPos());
                        }
                        bCharSpace = false;
                    } else if (operation == TagProcessor::OPERATION_NO_CHAR_SPACE) {
                        bCharSpace = false;
                    } else if (operation == TagProcessor::OPERATION_CHAR_SPACE) {
                        bCharSpace = true;
                    } else if (operation == TagProcessor::OPERATION_END_DRAW) {
                        break;
                    }

                    NW4R_POINTER_ASSERT_AT(423, context.str);
                    reader.Set(context.str);
                } else

                {
                    if (processor) {
                        SetAlpha(processor->mAlphaCtrl.alpha());
                        processor->mAlphaCtrl.mCharIndex++;
                    }

                    const f32 baseY = GetCursorY();

                    if (bUseLimit && (prevStreamPos != prevNewLinePos)) {
                        f32 baseX = GetCursorX();
                        f32 charSpace = bCharSpace ? GetCharSpace() : 0.0f;
                        f32 charWidth = IsWidthFixed() ? GetFixedWidth() : GetFont()->GetCharWidth(code) * GetScaleH();

                        if (baseX - xOrigin + charSpace + charWidth > mWidthLimit) {
                            code = '\n';
                            reader.Set(prevStreamPos);
                            continue;
                        }
                    }

                    if (bCharSpace) {
                        MoveCursorX(GetCharSpace());
                    }
                    bCharSpace = true;

                    {
                        const Font* pFont = GetFont();
                        const f32 adj = -pFont->GetBaselinePos() * GetScaleV();
                        MoveCursorY(adj);
                    }

                    CharWriter::Print(code);

                    if (processor) {
                        processor->mPreviousChar = code;
                    }

                    SetCursorY(baseY);
                }

                if (bUseLimit) {
                    prevStreamPos = reinterpret_cast< StreamType >(reader.GetCurrentPos());
                }

                code = reader.Next();
            }

            {
                const f32 width = GetCursorX() - context.xOrigin;
                textWidth = Max(textWidth, width);
            }

            if (IsDrawFlagSet(VERTICAL_ORIGIN_MASK, VERTICAL_ORIGIN_MIDDLE) || IsDrawFlagSet(VERTICAL_ORIGIN_MASK, VERTICAL_ORIGIN_BOTTOM)) {
                SetCursorY(orgCursorY);
            } else {
                MoveCursorY(yCursorAdj);
            }

            if (processor) {
                SetAlpha(alpha);
            }

            return textWidth;
        }

}
u8 CustomTagAlphaCtrl::alpha() const {
    if (mEnabled) {
        f32 value = mFadeSpeed * (mFrame - mWaitTime) - mCharIndex * mCharInterval;
        return 255.0f * std::max(0.0f, std::min(1.0f, value));
    }

    return 255;
}

void CustomTagProcessor::reset(const wchar_t* string) {
    if (mTextBox->mTextBuf != string) {
        return;
    }

    mSoundIndex = 0;
    mColorIndex = 0;
    mAlphaCtrl.mWaitTime = 0;
    mAlphaCtrl.mCharIndex = 0;
    mPreviousChar = 0;
}

