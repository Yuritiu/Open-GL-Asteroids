#include "UISystem.h"

#include <iostream>

bool UISystem::init(const std::string& fontPath, int fontSize)
{
	if (TTF_Init() == -1)
	{
		std::cerr << "Failed to initialise SDL_ttf: " << TTF_GetError() << std::endl;
		return false;
	}

	font = TTF_OpenFont(fontPath.c_str(), fontSize);

	if (!font)
	{
		std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
		return false;
	}

	std::cout << "UI font loaded successfully." << std::endl;
	return true;
}

void UISystem::cleanup()
{
	if (textTexture != 0)
	{
		glDeleteTextures(1, &textTexture);
		textTexture = 0;
	}

	if (font)
	{
		TTF_CloseFont(font);
		font = nullptr;
	}

	TTF_Quit();
}

void UISystem::update(int score, int lives, int wave, bool isGameOver)
{
	std::string newText;

	if (isGameOver)
	{
		newText = "GAME OVER\nFinal Score: " + std::to_string(score) + "\nPress R to Restart";
	}
	else
	{
		newText = "Score: " + std::to_string(score)
			+ "   Lives: " + std::to_string(lives)
			+ "   Wave: " + std::to_string(wave);
	}

	if (newText != currentText)
	{
		currentText = newText;
		createTextTexture(currentText);
	}
}

void UISystem::createTextTexture(const std::string& text)
{
	if (!font)
	{
		return;
	}

	SDL_Color white = { 255, 255, 255, 255 };

	SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(
		font,
		text.c_str(),
		white,
		800
	);

	if (!surface)
	{
		std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
		return;
	}

	SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat(
		surface,
		SDL_PIXELFORMAT_RGBA32,
		0
	);

	if (!formattedSurface)
	{
		std::cerr << "Failed to convert text surface: " << SDL_GetError() << std::endl;
		SDL_FreeSurface(surface);
		return;
	}

	if (textTexture != 0)
	{
		glDeleteTextures(1, &textTexture);
		textTexture = 0;
	}

	glGenTextures(1, &textTexture);
	glBindTexture(GL_TEXTURE_2D, textTexture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		formattedSurface->w,
		formattedSurface->h,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		formattedSurface->pixels
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	textWidth = formattedSurface->w;
	textHeight = formattedSurface->h;

	SDL_FreeSurface(formattedSurface);
	SDL_FreeSurface(surface);

	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint UISystem::getTextTexture() const
{
	return textTexture;
}

int UISystem::getTextWidth() const
{
	return textWidth;
}

int UISystem::getTextHeight() const
{
	return textHeight;
}