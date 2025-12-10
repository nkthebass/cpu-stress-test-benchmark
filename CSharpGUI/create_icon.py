from PIL import Image, ImageDraw, ImageFont
import os

# Create a high-resolution image for the icon (256x256 is standard for .ico)
sizes = [256, 128, 64, 48, 32, 16]
images = []

for size in sizes:
    # Create image with transparent background
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Define colors
    green = (76, 175, 80, 255)  # Material Design Green 500
    
    # Calculate dimensions
    padding = size // 8
    circle_bbox = [padding, padding, size - padding, size - padding]
    
    # Draw circle
    draw.ellipse(circle_bbox, fill=green)
    
    # Calculate font size based on icon size
    font_size = int(size * 0.6)
    
    # Try to use a system font, fallback to default if not available
    try:
        font = ImageFont.truetype("arial.ttf", font_size)
    except:
        try:
            font = ImageFont.truetype("C:\\Windows\\Fonts\\arial.ttf", font_size)
        except:
            font = ImageFont.load_default()
    
    # Get text size and position
    text = "C"
    
    # For newer Pillow versions, use textbbox
    try:
        bbox = draw.textbbox((0, 0), text, font=font)
        text_width = bbox[2] - bbox[0]
        text_height = bbox[3] - bbox[1]
    except:
        # Fallback for older versions
        text_width, text_height = draw.textsize(text, font=font)
    
    # Center the text
    text_x = (size - text_width) // 2
    text_y = (size - text_height) // 2 - size // 20  # Slight adjustment for better centering
    
    # Draw white text
    draw.text((text_x, text_y), text, fill=(255, 255, 255, 255), font=font)
    
    images.append(img)

# Save as .ico file with multiple sizes
output_path = os.path.join(os.path.dirname(__file__), 'app_icon.ico')
images[0].save(output_path, format='ICO', sizes=[(img.width, img.height) for img in images])

print(f"Icon created successfully: {output_path}")
