#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef struct point_struct Point;
typedef uint32_t RGBA;
struct point_struct
{
	int x;
	int y;
};

uint8_t BlendChannel(uint8_t background, uint8_t foreground, uint8_t alpha)
{
	return (uint8_t)((foreground * alpha + background * (255 - alpha)) / 255);
}

void PlotPixel(int width, int height, int channels, uint8_t *pixels, int x, int y, float brightness, RGBA color)
{
	uint8_t r = (color >> 24) & 0xFF;
	uint8_t g = (color >> 16) & 0xFF;
	uint8_t b = (color >> 8) & 0xFF;
	uint8_t a = color & 0xFF;
	if(x < 0 || x >= width || y < 0 || y >= height){/*Do nothing if point is outside image bounds*/}
	else
	{
		//Find pixel's index in our pixel array
		int pixelIndex = (y * width + x) * channels;
		//Find alpha
		uint8_t effectiveAlpha = (uint8_t)(a * brightness);
		if(channels >= 3)
		{
			if(channels == 4 && effectiveAlpha < 255)
			{
				pixels[pixelIndex + 0] = BlendChannel(pixels[pixelIndex + 0], r, effectiveAlpha);
				pixels[pixelIndex + 1] = BlendChannel(pixels[pixelIndex + 1], g, effectiveAlpha);
				pixels[pixelIndex + 2] = BlendChannel(pixels[pixelIndex + 2], b, effectiveAlpha);
				if(channels == 4)
				{
					pixels[pixelIndex + 3] = 255; // Keep full opacity for blended pixels
				}
			}
			else 
			{
				pixels[pixelIndex + 0] = r;
				pixels[pixelIndex + 1] = g;
				pixels[pixelIndex + 2] = b;
				if(channels == 4)
				{
					pixels[pixelIndex + 3] = a;
				}
			}
		}
		else if(channels == 1)
		{
			//Calculate the gray level of the pixel
			uint8_t gray = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
			pixels[pixelIndex] = BlendChannel(pixels[pixelIndex], gray, effectiveAlpha);
		}
	}
}

void DrawLine(int width, int height, int channels, uint8_t *pixels, Point start, Point end, RGBA color)
{
	//Extract colors and point coordinates
	uint8_t r = (color >> 24) & 0xFF;
	uint8_t g = (color >> 16) & 0xFF;
	uint8_t b = (color >> 8) & 0xFF;
	uint8_t a = color & 0xFF;

	int x0 = start.x, y0 = start.y;
	int x1 = end.x, y1 = end.y;
	
	//Find the greater difference
	int steep = abs(y1 - y0) > abs(x1 - x0);
	
        //Swap x and y if y has a greater difference than x 
	if(steep){int tmp = x0; x0 = y0; y0 = tmp;tmp = x1; x1 = y1; y1 = tmp;}
	//Set the smaller x value to x0  
	if(x0 > x1){int tmp = x0; x0 = x1; x1 = tmp;tmp = y0; y0 = y1; y1 = tmp;}
	//Find new differences
	float dx = x1 - x0;
	float dy = y1 - y0;
	float gradient = (dx == 0) ? 1.0f : dy / dx;
	// First endpoint
	float xend = round(x0);
	float yend = y0 + gradient * (xend - x0);
	float xgap = 1.0f - ((x0 + 0.5f) - floor(x0 + 0.5f));
	int xpxl1 = xend;
	int ypxl1 = floor(yend);

	if(steep)
	{
		PlotPixel(width, height, channels, pixels, ypxl1, xpxl1, 1.0f - (yend - floor(yend)) * xgap, color);
		PlotPixel(width, height, channels, pixels, ypxl1 + 1, xpxl1, (yend - floor(yend)) * xgap, color);
	}
	else
	{
		PlotPixel(width, height, channels, pixels, xpxl1, ypxl1, 1.0f - (yend - floor(yend)) * xgap, color);
		PlotPixel(width, height, channels, pixels, xpxl1, ypxl1 + 1, (yend - floor(yend)) * xgap, color);
	}
	
	float intery = yend + gradient;
	
	//Second endpoint
	xend = round(x1);
	yend = y1 + gradient * (xend - x1);
	xgap = (x1 + 0.5f) - floor(x1 + 0.5f);
	int xpxl2 = xend;
	int ypxl2 = floor(yend);
	if(steep)
	{
		PlotPixel(width, height, channels, pixels, ypxl2, xpxl2, 1.0f - (yend - floor(yend)) * xgap, color);
		PlotPixel(width, height, channels, pixels, ypxl2 + 1, xpxl2, (yend - floor(yend)) * xgap, color);
	}
	else
	{
		PlotPixel(width, height, channels, pixels, xpxl2, ypxl2, 1.0f - (yend - floor(yend)) * xgap, color);
		PlotPixel(width, height, channels, pixels, xpxl2, ypxl2 + 1, (yend - floor(yend)) * xgap, color);
	}
	
	//Move between endpoints
	for(int x = xpxl1 + 1; x < xpxl2; x++)
	{
		if(steep)
		{
			PlotPixel(width, height, channels, pixels, floor(intery), x, 1.0f - (intery - floor(intery)), color);
			PlotPixel(width, height, channels, pixels, floor(intery) + 1, x, intery - floor(intery), color);
		}
		else
		{
			PlotPixel(width, height, channels, pixels, x, floor(intery), 1.0f - (intery - floor(intery)), color);
			PlotPixel(width, height, channels, pixels, x, floor(intery) + 1, intery - floor(intery), color);
		}
		intery += gradient;
	}
}

void DrawCircleOutline(int width, int height, int channels, uint8_t *pixels, 
                      Point center, int radius, RGBA color)
{
    int cx = center.x;
    int cy = center.y;
    float outer = radius + 0.5f;
    float inner = radius - 0.5f;
    const int samples = 4; // 4x4 super-sampling

    // Extract color components
    uint8_t r = (color >> 24) & 0xFF;
    uint8_t g = (color >> 16) & 0xFF;
    uint8_t b = (color >> 8) & 0xFF;
    uint8_t a = color & 0xFF;

    // Calculate bounding box
    int minX = cx - radius - 1;
    int maxX = cx + radius + 1;
    int minY = cy - radius - 1;
    int maxY = cy + radius + 1;

    // Clip to image bounds
    minX = (minX < 0) ? 0 : minX;
    maxX = (maxX >= width) ? width - 1 : maxX;
    minY = (minY < 0) ? 0 : minY;
    maxY = (maxY >= height) ? height - 1 : maxY;

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            float coverage = 0.0f;
            float sampleStep = 1.0f / samples;
            
            // Super-sampling loop
            for (float sy = -0.5f + sampleStep/2; sy < 0.5f; sy += sampleStep) {
                for (float sx = -0.5f + sampleStep/2; sx < 0.5f; sx += sampleStep) {
                    float dx = (x - cx) + sx;
                    float dy = (y - cy) + sy;
                    float distance = sqrtf(dx*dx + dy*dy);
                    
                    // Only draw pixels between inner and outer radius
                    if (distance >= inner && distance <= outer) {
                        coverage += 1.0f - fabsf(distance - radius);
                    }
                }
            }
            
            coverage /= (samples * samples);
            if (coverage > 0.0f) {
                PlotPixel(width, height, channels, pixels, x, y, coverage, color);
            }
        }
    }
}

void DrawFilledCircle(int width, int height, int channels, uint8_t *pixels, Point center, int radius, RGBA color)
{
    int cx = center.x;
    int cy = center.y;
    float radiusSquared = radius * radius;
    const int samples = 4; // 4x4 super-sampling
    
    // Extract color components
    uint8_t r = (color >> 24) & 0xFF;
    uint8_t g = (color >> 16) & 0xFF;
    uint8_t b = (color >> 8) & 0xFF;
    uint8_t a = color & 0xFF;

    // Draw the circle with super-sampling anti-aliasing
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            int px = cx + x;
            int py = cy + y;
            
            if (px >= 0 && px < width && py >= 0 && py < height) {
                float coverage = 0.0f;
                float sampleStep = 1.0f / samples;
                
                // Super-sampling loop
                for (float sy = -0.5f + sampleStep/2; sy < 0.5f; sy += sampleStep) {
                    for (float sx = -0.5f + sampleStep/2; sx < 0.5f; sx += sampleStep) {
                        float dx = x + sx;
                        float dy = y + sy;
                        float distance = sqrt(dx*dx + dy*dy);
                        
                        if (distance <= radius + 0.5f) {
                            if (distance <= radius - 0.5f) {
                                coverage += 1.0f; // Fully inside
                            }
                            else {
                                // Partial coverage at edge
                                coverage += 1.0f - (distance - (radius - 0.5f));
                            }
                        }
                    }
                }
                
                coverage /= (samples * samples);
                if (coverage > 0.0f) {
                    PlotPixel(width, height, channels, pixels, px, py, coverage, color);
                }
            }
        }
    }
}

int main()
{
	char *outputFile = "output.png";
	int width = 512;
	int height = 512;
	int channels = 4; 

	//Create image with white background
	uint8_t *pixels = malloc(width * height * channels);
	for(int i = 0; i < width * height * channels; i += channels)
	{
		pixels[i + 0] = 255; // R
		pixels[i + 1] = 255; // G
		pixels[i + 2] = 255; // B
		if(channels == 4)
		{
			pixels[i + 3] = 255; // A
		}
	}
	Point start = {100, 100};
	Point end = {400, 400};
	DrawLine(width, height, channels, pixels, start, end, 0xFF0000FF); // Red line

	start = (Point){50, 450};
	end = (Point){450, 50};
	DrawLine(width, height, channels, pixels, start, end, 0x00FF00FF); // Green line

	Point circleCenter = {256, 256};
	DrawCircleOutline(width, height, channels, pixels, circleCenter, 150, 0x0000FFFF); // Blue circle
	//Write image to file
	stbi_write_png(outputFile, width, height, channels, pixels, width * channels);
}
