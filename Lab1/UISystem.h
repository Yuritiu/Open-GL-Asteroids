#pragma once

#include <string>
#include <SDL_ttf.h>
#include <GL/glew.h>

class UISystem
{
public:
	bool init(const std::string& fontPath, int fontSize);
	void cleanup();

	void update(int score, int lives, int wave, bool isGameOver);

	GLuint getTextTexture() const;
	int getTextWidth() const;
	int getTextHeight() const;

private:
	void createTextTexture(const std::string& text);

	TTF_Font* font = nullptr;

	GLuint textTexture = 0;
	int textWidth = 0;
	int textHeight = 0;

	std::string currentText = "";
};