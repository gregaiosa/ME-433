try:
    from PIL import Image
    img = Image.open('images/player.png')
    img = img.convert("RGBA")
    datas = img.getdata()
    newData = []
    for item in datas:
        if item[0] > 240 and item[1] > 240 and item[2] > 240:
            newData.append((255, 255, 255, 0))
        else:
            newData.append(item)
    img.putdata(newData)
    
    # Let's also resize it to be 40x40 or 50x50 so it's not giant if the generator made it 512x512
    img = img.resize((40, 50), Image.Resampling.LANCZOS)
    img.save('images/player.png', "PNG")
except ImportError:
    print("PIL not installed, using original image.")
