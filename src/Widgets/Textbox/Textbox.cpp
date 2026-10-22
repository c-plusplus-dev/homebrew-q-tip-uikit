//
// Created by Ayaan on 2026-08-30.
//

#include "QTipUIKit/Widgets/Textbox/Textbox.h"

#include <algorithm>
#include <iostream>

#include "Q-Tip/Graphics/RenderTarget.h"

#include <SDL3/SDL.h>

#include "QTipUIKit/UIKitMod.h"
#include "QTipUIKit/UIObject.h"
#include "QTipUIKit/Internal/Helpers.h"

using namespace QTip;


// ============================================================
// Construction
// ============================================================

Textbox::Textbox(
    float x,
    float y,
    float width,
    float height,
    const std::optional<Font>& font
) : _font(Detail::defaultFontPath(), 16) {

    init({x, y, width, height}, font);
}


Textbox::Textbox(
    const Rect rect,
    const std::optional<Font>& font
) : _font(Detail::defaultFontPath(), 16) {

    init(rect, font);
}

void Textbox::init(Rect rect, const std::optional<Font>& font) {
    if (font.has_value()) {
        _font.destroy();
        _font = font.value();
    }

    _fontHeight = _font.getFontHeight();

    _rect = rect;
    _preferredRect = _rect;

    _textCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
    if (!_textCursor)
        UIKitMod::instance()->log(fmt("Failed to create text cursor: %s", SDL_GetError()), LOG_WARNING);

    setMinHeight();
}

Textbox::~Textbox() {
    if (_textCursor) {
        SDL_DestroyCursor(_textCursor);
    }
}


// ============================================================
// Rendering
// ============================================================

void Textbox::renderImpl(Window& window) {
    Renderer& renderer = window.getRenderer();

    RenderTarget target(window, _rect.size);

    renderer.setTarget(target);

    renderer.setRenderColor({40, 40, 40});
    renderer.renderRoundedRect(
        {0, 0, _rect.size},
        10
    );

    /*
     * Everything inside the textbox is drawn relative to
     * the textbox's local coordinate system.
     */
    const float textX = 5.0f - _scroll.x;
    const float textY = 5.0f - _scroll.y;

    /*
     * Render every line independently.
     *
     * This is important because a normal renderText() call
     * shouldn't be expected to understand '\n'.
     */
    size_t lineNumber = 0;
    size_t lineStart = 0;

    while (lineStart <= _text.size()) {
        const size_t newlinePosition =
            _text.find('\n', lineStart);

        const size_t lineEnd =
            newlinePosition == std::string::npos
                ? _text.size()
                : newlinePosition;

        const std::string line =
            _text.substr(
                lineStart,
                lineEnd - lineStart
            );

        const float y =
            textY + static_cast<float>(lineNumber) * _fontHeight;

        /*
         * Only render lines that could be visible.
         *
         * This isn't required, but avoids asking the renderer
         * to draw hundreds of lines that are outside the box.
         */
        if (y + _fontHeight >= 0 &&
            y <= _rect.size.y) {

            renderer.renderText(
                _font,
                line.c_str(),
                textX,
                y,
                Color::white
            );
        }

        if (newlinePosition == std::string::npos)
            break;

        lineStart = newlinePosition + 1;
        ++lineNumber;
    }


    // ========================================================
    // Caret
    // ========================================================

    if (_active) {
        const uint64_t ticks = SDL_GetTicks();

        if ((ticks - _blinkTimer) / 500 % 2 == 0) {
            Rect caret{};

            caret.origin.x =
                textX + getCaretX();

            caret.origin.y =
                textY + getCaretY();

            caret.size.x = 2;
            caret.size.y = _fontHeight;

            renderer.setRenderColor({255, 255, 255});
            renderer.renderRect(caret);
        }
    }
    if (_active) {
        SDL_Rect area{
            static_cast<int>(
                _rect.origin.x +
                5.0f -
                _scroll.x +
                getCaretX()
            ),
            static_cast<int>(
                _rect.origin.y +
                5.0f -
                _scroll.y +
                getCaretY()
            ),
            2,
            static_cast<int>(_fontHeight)
        };

        SDL_SetTextInputArea(
            Textbox::window(window),
            &area,
            0
        );
    }


    // ========================================================
    // Finish target texture
    // ========================================================

    renderer.resetTarget();

    renderer.renderTexture(target, Rect::zero, _rect);
}


// ============================================================
// Event handling
// ============================================================

bool Textbox::handleEvent(const SDL_Event& event) {
    switch (event.type) {

    // --------------------------------------------------------
    // Mouse click
    // --------------------------------------------------------

    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        if (event.button.button != SDL_BUTTON_LEFT)
            break;

        if (_rect.isPointInside({
            event.button.x,
            event.button.y
        })) {

            _active = true;

            /*
             * Convert the mouse position into textbox-local
             * coordinates before finding the caret.
             */
            const float mouseX =
                event.button.x - _rect.origin.x;

            const float mouseY =
                event.button.y - _rect.origin.y;

            _caretPosition =
                getCaretPosition(
                    mouseX,
                    mouseY
                );

            _preferredColumn =
                getCurrentColumn();

            _blinkTimer = SDL_GetTicks();
            return false;
        } else {
            _active = false;
        }

        break;
    }


    // --------------------------------------------------------
    // Mouse wheel
    // --------------------------------------------------------

    case SDL_EVENT_MOUSE_WHEEL: {
        if (!_rect.isPointInside({
            event.wheel.mouse_x,
            event.wheel.mouse_y
        })) {
            break;
        }

        Point maxScroll = _scrollMaxSize;

        float scrollX =
            event.wheel.x *
            event.wheel.x *
            (event.wheel.x >= 0
                ? 1.0f
                : -1.0f);

        float scrollY =
            event.wheel.y *
            event.wheel.y *
            (event.wheel.y >= 0
                ? 1.0f
                : -1.0f);

        /*
         * If there is only vertical scrolling, horizontal wheel
         * input can still scroll vertically.
         */
        if (needsScrollX()) {
            if (needsScrollY())
                _scroll.x += scrollX;
            else
                _scroll.x += scrollY;
        }

        if (needsScrollY()) {
            _scroll.y -= scrollY;
        }

        _scroll.x = std::clamp(
            _scroll.x,
            0.0f,
            maxScroll.x
        );

        _scroll.y = std::clamp(
            _scroll.y,
            0.0f,
            maxScroll.y
        );

        return false;
    }


    case SDL_EVENT_MOUSE_MOTION: {
        if (_rect.isPointInside({event.motion.x, event.motion.y})) {
            if (_textCursor)
                SDL_SetCursor(_textCursor);
            return false;
        } else {
            SDL_SetCursor(SDL_GetDefaultCursor());
        }

        break;
    }


    // --------------------------------------------------------
    // Keyboard
    // --------------------------------------------------------

    case SDL_EVENT_KEY_DOWN: {
        if (!_active)
            break;

        switch (event.key.key) {

        // ----------------------------------------------------
        // Backspace
        // ----------------------------------------------------

        case SDLK_BACKSPACE:
            deleteCharacter();
            break;


        // ----------------------------------------------------
        // Left
        // ----------------------------------------------------

        case SDLK_LEFT:
            if (_caretPosition > 0) {
                --_caretPosition;
            }

            _preferredColumn = getCurrentColumn();

            break;


        // ----------------------------------------------------
        // Right
        // ----------------------------------------------------

        case SDLK_RIGHT:
            if (_caretPosition < getLineEndCaret(getCurrentLine())) {
                ++_caretPosition;
            } else {
                /*
                 * If we're at the end of a line and there is a
                 * newline after it, move across the newline.
                 */
                const size_t bytePosition = getBytePosition();

                if (bytePosition < _text.size() &&
                    _text[bytePosition] == '\n') {
                    ++_caretPosition;
                }
            }

            _preferredColumn = getCurrentColumn();

            break;


        // ----------------------------------------------------
        // Up
        // ----------------------------------------------------

        case SDLK_UP: {
            const size_t currentLine =
                getCurrentLine();

            if (currentLine > 0) {
                _caretPosition =
                    getCaretPositionFromLineColumn(
                        currentLine - 1,
                        _preferredColumn
                    );
            }

            break;
        }


        // ----------------------------------------------------
        // Down
        // ----------------------------------------------------

        case SDLK_DOWN: {
            const size_t currentLine =
                getCurrentLine();

            const size_t lineCount =
                getLineCount();

            if (currentLine + 1 < lineCount) {
                _caretPosition =
                    getCaretPositionFromLineColumn(
                        currentLine + 1,
                        _preferredColumn
                    );
            }

            break;
        }


        // ----------------------------------------------------
        // Home
        // ----------------------------------------------------

        case SDLK_HOME:
            _caretPosition =
                getLineStartCaret(
                    getCurrentLine()
                );

            _preferredColumn = 0;

            break;


        // ----------------------------------------------------
        // End
        // ----------------------------------------------------

        case SDLK_END:
            _caretPosition =
                getLineEndCaret(
                    getCurrentLine()
                );

            _preferredColumn =
                getCurrentColumn();

            break;


        // ----------------------------------------------------
        // Enter
        // ----------------------------------------------------

        case SDLK_RETURN:
            type("\n");
            break;


        default:
            break;
        }

        _blinkTimer = SDL_GetTicks();

        scrollCaretIntoView();

        break;
    }


    // --------------------------------------------------------
    // Text input
    // --------------------------------------------------------

    case SDL_EVENT_TEXT_INPUT: {
        if (!_active)
            break;

        type(event.text.text);

        _preferredColumn = getCurrentColumn();

        _blinkTimer = SDL_GetTicks();

        scrollCaretIntoView();

        break;
    }


    default:
        break;
    }
    auto position = Detail::eventPosition(event);
    if (position != QTip::Point{-1, -1}) {
        return !_rect.isPointInside(position);
    }
    return true;
}


// ============================================================
// Active state
// ============================================================

void Textbox::setActive(bool active) {
    _active = active;

    if (active) {
        _blinkTimer = SDL_GetTicks();
    }
}


bool Textbox::getActive() const {
    return _active;
}


// ============================================================
// Text input
// ============================================================

void Textbox::type(const std::string_view text) {
    if (text.empty())
        return;

    const size_t bytePosition =
        getBytePosition();

    _text.insert(
        bytePosition,
        text
    );

    /*
     * Count UTF-8 codepoints.
     *
     * Newline is a normal one-byte character, so it naturally
     * counts as one caret position too.
     */
    size_t addedCharacters = 0;

    for (size_t i = 0; i < text.size();) {
        const auto c =
            static_cast<unsigned char>(text[i]);

        if ((c & 0b10000000) == 0) {
            i += 1;
        }
        else if ((c & 0b11100000) == 0b11000000) {
            i += 2;
        }
        else if ((c & 0b11110000) == 0b11100000) {
            i += 3;
        }
        else if ((c & 0b11111000) == 0b11110000) {
            i += 4;
        }
        else {
            /*
             * Invalid UTF-8 byte.
             *
             * Treat it as one byte rather than accidentally
             * walking past the end of the string.
             */
            i += 1;
        }

        ++addedCharacters;
    }

    _caretPosition += addedCharacters;

    _preferredColumn = getCurrentColumn();

    scrollCaretIntoView();
}


// ============================================================
// Text getters/setters
// ============================================================

std::string Textbox::getText() const {
    return _text;
}


void Textbox::setText(const std::string& text) {
    _text = text;

    /*
     * Put the caret at the end of the new text.
     */
    _caretPosition = 0;

    size_t bytePosition = 0;

    while (bytePosition < _text.size()) {
        bytePosition =
            getNextCharacterBytePosition(
                bytePosition
            );

        ++_caretPosition;
    }

    _preferredColumn = getCurrentColumn();

    scrollCaretIntoView();
}

Point Textbox::minimumSize() const {
    return {_font.getTextWidth(getText()), _fontHeight * 2}; // later when flags are added,
                                                                      // this will be dynamic.
}

Point Textbox::preferredSize() const {
    return _preferredRect.size;
}

void Textbox::resize(Point size) {
    _rect.size = size;

    setMinHeight();

    _preferredRect.size = _rect.size;

    _scroll = {0, 0};
    scrollCaretIntoView();
}

void Textbox::reposition(Point position) {
    _rect.origin = position;
    _preferredRect.origin = position;
}

void Textbox::setRect(Rect rect) {
    _rect = rect;

    setMinHeight();

    _scroll = {0, 0};
    scrollCaretIntoView();
}

const Rect& Textbox::rect() {
    return _rect;
}


// ============================================================
// Minimum height
// ============================================================

void Textbox::setMinHeight() {
    if (_rect.size.y < _fontHeight + 10.0f) {
        _rect.size.y = _fontHeight + 10.0f;
        _preferredRect.size.y = _rect.size.y;
    }
}


// ============================================================
// Delete character
// ============================================================

void Textbox::deleteCharacter() {
    const size_t bytePosition =
        getBytePosition();

    if (bytePosition == 0)
        return;

    const size_t start =
        getPreviousCharacterBytePosition(
            bytePosition
        );

    _text.erase(
        start,
        bytePosition - start
    );

    --_caretPosition;

    _preferredColumn = getCurrentColumn();

    scrollCaretIntoView();
}


// ============================================================
// Scroll requirements
// ============================================================

bool Textbox::needsScrollX() const {
    const Point size = _textSize;

    return size.x >
           _rect.size.x - 10.0f;
}


bool Textbox::needsScrollY() const {
    const Point size = _textSize;

    return size.y >
           _rect.size.y - 10.0f;
}


// ============================================================
// Get byte position from caret position
// ============================================================

size_t Textbox::getBytePosition() const {
    size_t bytePosition = 0;

    for (size_t i = 0;
         i < _caretPosition &&
         bytePosition < _text.size();
         ++i) {

        bytePosition =
            getNextCharacterBytePosition(
                bytePosition
            );
    }

    return bytePosition;
}


// ============================================================
// UTF-8 helpers
// ============================================================

size_t Textbox::getNextCharacterBytePosition(
    size_t bytePosition
) const {
    if (bytePosition >= _text.size())
        return _text.size();

    const auto c =
        static_cast<unsigned char>(
            _text[bytePosition]
        );

    if ((c & 0b10000000) == 0)
        return bytePosition + 1;

    if ((c & 0b11100000) == 0b11000000)
        return std::min(
            bytePosition + 2,
            _text.size()
        );

    if ((c & 0b11110000) == 0b11100000)
        return std::min(
            bytePosition + 3,
            _text.size()
        );

    if ((c & 0b11111000) == 0b11110000)
        return std::min(
            bytePosition + 4,
            _text.size()
        );

    return bytePosition + 1;
}


size_t Textbox::getPreviousCharacterBytePosition(
    size_t bytePosition
) const {
    if (bytePosition == 0)
        return 0;

    size_t start = bytePosition - 1;

    while (start > 0 &&
           (static_cast<unsigned char>(
               _text[start]
           ) & 0b11000000) == 0b10000000) {

        --start;
    }

    return start;
}


// ============================================================
// Lines
// ============================================================

size_t Textbox::getLineCount() const {
    size_t count = 1;

    for (char c : _text) {
        if (c == '\n')
            ++count;
    }

    return count;
}


size_t Textbox::getCurrentLine() const {
    const size_t bytePosition =
        getBytePosition();

    size_t line = 0;

    for (size_t i = 0;
         i < bytePosition;
         ++i) {

        if (_text[i] == '\n')
            ++line;
    }

    return line;
}


size_t Textbox::getLineStartByte(
    size_t line
) const {
    if (line == 0)
        return 0;

    size_t currentLine = 0;

    for (size_t i = 0;
         i < _text.size();
         ++i) {

        if (_text[i] == '\n') {
            ++currentLine;

            if (currentLine == line)
                return i + 1;
        }
    }

    return _text.size();
}


size_t Textbox::getLineEndByte(
    size_t line
) const {
    const size_t start =
        getLineStartByte(line);

    const size_t newline =
        _text.find('\n', start);

    if (newline == std::string::npos)
        return _text.size();

    return newline;
}


size_t Textbox::getLineStartCaret(
    size_t line
) const {
    const size_t targetByte =
        getLineStartByte(line);

    size_t caret = 0;
    size_t byte = 0;

    while (byte < targetByte) {
        byte =
            getNextCharacterBytePosition(byte);

        ++caret;
    }

    return caret;
}


size_t Textbox::getLineEndCaret(
    size_t line
) const {
    const size_t targetByte =
        getLineEndByte(line);

    size_t caret = 0;
    size_t byte = 0;

    while (byte < targetByte) {
        byte =
            getNextCharacterBytePosition(byte);

        ++caret;
    }

    return caret;
}


size_t Textbox::getCurrentColumn() const {
    const size_t line =
        getCurrentLine();

    const size_t lineStart =
        getLineStartCaret(line);

    return _caretPosition - lineStart;
}


size_t Textbox::getCaretPositionFromLineColumn(
    size_t line,
    size_t column
) const {
    const size_t start =
        getLineStartCaret(line);

    const size_t end =
        getLineEndCaret(line);

    return std::min(
        start + column,
        end
    );
}


size_t Textbox::getLineColumnFromCaret(
    size_t caretPosition
) const {
    size_t byte = 0;
    size_t lineStartCaret = 0;

    for (size_t caret = 0;
         caret < caretPosition &&
         byte < _text.size();
         ++caret) {

        if (_text[byte] == '\n') {
            lineStartCaret = caret + 1;
        }

        byte =
            getNextCharacterBytePosition(byte);
    }

    return caretPosition - lineStartCaret;
}


std::string Textbox::getLine(
    size_t line
) const {
    const size_t start =
        getLineStartByte(line);

    const size_t end =
        getLineEndByte(line);

    return _text.substr(
        start,
        end - start
    );
}


std::string Textbox::getLineBeforeCaret() const {
    const size_t line =
        getCurrentLine();

    const size_t start =
        getLineStartByte(line);

    const size_t bytePosition =
        getBytePosition();

    return _text.substr(
        start,
        bytePosition - start
    );
}


// ============================================================
// Caret position
// ============================================================

float Textbox::getCaretX() const {
    const std::string line =
        getLineBeforeCaret();

    return _font.getTextWidth(line);
}


float Textbox::getCaretY() const {
    const size_t line =
        getCurrentLine();

    return static_cast<float>(line) *
           _fontHeight;
}


// ============================================================
// Mouse → caret
// ============================================================

size_t Textbox::getCaretPosition(
    float mouseX,
    float mouseY
) const {
    /*
     * Convert the mouse position into the coordinate system
     * of the text.
     */
    const float textX =
        5.0f - _scroll.x;

    const float textY =
        5.0f - _scroll.y;

    /*
     * Figure out which line was clicked.
     */
    const float relativeY =
        mouseY - textY;

    size_t line = 0;

    if (relativeY > 0) {
        line = static_cast<size_t>(
            relativeY / _fontHeight
        );
    }

    line = std::min(
        line,
        getLineCount() - 1
    );

    const std::string lineText =
        getLine(line);

    const size_t lineStartCaret =
        getLineStartCaret(line);

    /*
     * Mouse X relative to the beginning of the line.
     */
    const float relativeX =
        mouseX - textX;

    if (relativeX <= 0)
        return lineStartCaret;

    /*
     * Walk through UTF-8 characters and find the closest
     * caret position using the midpoint between characters.
     */
    size_t bytePosition =
        getLineStartByte(line);

    size_t caretPosition =
        lineStartCaret;

    const size_t lineEnd =
        getLineEndByte(line);

    while (bytePosition < lineEnd) {
        const size_t nextBytePosition =
            getNextCharacterBytePosition(
                bytePosition
            );

        const float currentX =
            _font.getTextWidth(
                _text.substr(
                    getLineStartByte(line),
                    bytePosition -
                    getLineStartByte(line)
                )
            );

        const float nextX =
            _font.getTextWidth(
                _text.substr(
                    getLineStartByte(line),
                    nextBytePosition -
                    getLineStartByte(line)
                )
            );

        const float midpoint =
            (currentX + nextX) / 2.0f;

        if (relativeX < midpoint)
            return caretPosition;

        bytePosition =
            nextBytePosition;

        ++caretPosition;
    }

    return getLineEndCaret(line);
}


// ============================================================
// Keep caret visible
// ============================================================

void Textbox::scrollCaretIntoView() {
    const float caretX =
        getCaretX();

    const float caretY =
        getCaretY();

    const float visibleWidth =
        _rect.size.x - 10.0f;

    const float visibleHeight =
        _rect.size.y - 10.0f;


    // --------------------------------------------------------
    // Horizontal
    // --------------------------------------------------------

    if (caretX < _scroll.x) {
        _scroll.x = caretX;
    }
    else if (caretX > _scroll.x + visibleWidth) {
        _scroll.x =
            caretX - visibleWidth;
    }


    // --------------------------------------------------------
    // Vertical
    // --------------------------------------------------------

    if (caretY < _scroll.y) {
        _scroll.y = caretY;
    }
    else if (
        caretY + _fontHeight >
        _scroll.y + visibleHeight
    ) {
        _scroll.y =
            caretY +
            _fontHeight -
            visibleHeight;
    }


    // --------------------------------------------------------
    // Clamp
    // --------------------------------------------------------

    const Point maxScroll =
        _scrollMaxSize;

    _scroll.x = std::clamp(
        _scroll.x,
        0.0f,
        maxScroll.x
    );

    _scroll.y = std::clamp(
        _scroll.y,
        0.0f,
        maxScroll.y
    );
}
