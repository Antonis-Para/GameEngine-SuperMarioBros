# CS-454. Development of Intelligent Interfaces and Games

---

## Implementation of Super Mario

---

## Team
- Paragioudakis Antonis csd4022@csd.uoc.gr
- Pantelakis Giorgos Stavros csd4017@csd.uoc.gr
- Raptakis Michail csd4101@csd.uoc.gr

---

## How to run
- Open Visual Studio 2019 and create a new empty project. It's important to check the **"Place solution and project in the same directory"** box
- Go to Tools -> NuGet Package Manager -> Manage NuGet Packages for Solution. Click Browse and then Install Allegro for your project
- Download the code of this project as a Zip file and extract the contents of GameEngine-SuperMarioBros-master inside your project. (where the .sln file is)
- In the Solution Explorer, right click on the Source Files -> Add -> Existing Item and add all the .cpp files in \Engine\Src.
- In the Solution Explorer, right click on the project name -> Properties -> Allegro 5. Select "All configurations" and change the values of the following Addons to **yes**:
    + Image Addon
    + Truetype Font Addon
    + Primitives Addon
    + Audio Addon
    + Audio Codec Addon
    + Font Addon
- Run the project (F5 or Ctrl+F5)


---

## About Implementation
The implementation of the work was done in the environment of Microsoft Visual Studio 2019.  
For the graphics the Allegro5 library was used, which we used through the NuGet Package Manager  
Microsoft Visual Studio 2019 and the activation of the necessary Addons from the properties of  
the project (Image, Truetype Font, Primitives, Audio, Audio Codec Addons). To run the game, a  
new project must be created, with the above features, and the code imported. Note that in the  
config.ini file the necessary changes must be made to the path of the variables depending on  
the structure of the project.

---

### About Allegro

[Allegro Oficial repository](https://github.com/liballeg/allegro5)  
[Allegro Oficial wiki](https://github.com/liballeg/allegro_wiki/wiki)
