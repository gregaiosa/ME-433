from PIL import Image
try:
    img = Image.open('/home/gaiosa/.gemini/antigravity/brain/97a745bf-0be7-42bd-9035-e100f5833c34/enemy_sprite_1778901560069.png')
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] > 240 and item[1] > 240 and item[2] > 240:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    bbox = img.getbbox()
    if bbox:
        img = img.crop(bbox)
    img = img.resize((40, 40), Image.Resampling.LANCZOS)
    img.save('images/enemy.png', "PNG")
    print("Enemy image processed successfully.")
except Exception as e:
    print(f"Error processing image: {e}")
