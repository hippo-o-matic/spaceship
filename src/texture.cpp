#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"

std::vector<Texture> Texture::loaded;

Texture loadTexture(std::string path) {
	// Check to see if this texture has already been loaded
	auto check = std::find_if(
		Texture::loaded.begin(),
		Texture::loaded.end(),
		[path](Texture t){ return t.path == path; }
	);
	if(check != Texture::loaded.end()) {
		return *check;
	}

	// If not, begin creating a new texture
    Texture tex{0, "default", path};

	// Generate 1 id for this texture
    glGenTextures(1, &tex.glID);

	// Set image loading settings
	stbi_set_flip_vertically_on_load(true); // For some reason, the default stbi behavior is to give us images backwards, this will fix that

    int nrComponents;
    unsigned char *data = stbi_load(path.c_str(), &tex.width, &tex.height, &nrComponents, 0);
    if (data) {
        GLenum format = 4;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
		    format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, tex.glID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, tex.width, tex.height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Cleanup
		glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);

    } else {
		GLenum err;
		if((err = glGetError()) != GL_NO_ERROR){
			log("GL Error: " + err, WARN);
		}
		log("Texture failed to load at path: \"" + path + "\"", WARN);
    }

	stbi_image_free(data);
	Texture::loaded.push_back(tex);
    return tex;
}

TexMap::TexMap(std::string path, int tile_width, int tile_height) : tile_width(tile_width), tile_height(tile_height) {
	texture = loadTexture(path);
	
	int margin;
	margin = texture.width % tile_width;
	columns = (texture.width - margin) / tile_width;

	margin = texture.height % tile_height;
	rows = (texture.height - margin) / tile_height;
}

unsigned TexMap::getGLID() {
	return texture.glID;
}

std::string TexMap::getPath() {
	return texture.path;
}

std::vector<glm::vec2> TexMap::getTileCoords(unsigned index) {
	// Get texcoords for these tiles
	// Find the row and column the index points to
	int column = index % columns;
	int row = index / columns;
	
	// Find the first texture coordinate
	glm::vec2 BL = glm::vec2(
		column * (float)tile_width / (float)texture.width,
		row * (float)tile_height / (float)texture.height
	);

	std::vector<glm::vec2> texcoords = {
		BL, // Bottom left
		glm::vec2(BL.x + (float)tile_width / (float)texture.width, BL.y),					 						// Top left
		glm::vec2(BL.x + (float)tile_width / (float)texture.width, BL.y + (float)tile_height / (float)texture.height),// Top Right
		glm::vec2(BL.x, BL.y + (float)tile_height / (float)texture.height) 											// Bottom Right
	};

	return texcoords;
}

Texture TexMap::getTileTexture(unsigned index) {
	std::vector<glm::vec2> tcoords = getTileCoords(index);
	return Texture {
		texture.glID,
		texture.type,
		texture.path,
		texture.width,
		texture.height,
		{tcoords[0], tcoords[2]}
	};
}