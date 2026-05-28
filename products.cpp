#include <iostream>
#include <string>

using namespace std;


//абстрактный класс - Книга
struct Book
{
    virtual ~Book() {}
    virtual void read() const = 0; //метод чтения книги
};


struct Movie //класс для фильмов
{
    virtual ~Movie() {}
    virtual void watch() const = 0; // Метод общего действия для фильмов
};



//детские товары
struct FairyTale : Book //класс сказок
{ 
    void read() const override
    {
        cout << "Книга. Чтение сказки" << endl;
    }
};

struct Cartoon : Movie //класс мультиков
{ 
    void watch() const override
    {
        cout << "Фильм. Просмотр мультика" << endl;
    }
};

//учебные товары
struct TextBook : Book //класс учебника
{ 
    void read() const override {
        cout << "Книга. Чтение учебнника" << endl;
    }
};

struct DocumentaryMovie : Movie //класс документальных фильмов
{ 
    void watch() const override {
        cout << "Фильм. Просмотр документального фильма" << endl;
    }
};



struct AbstractShopFactory //класс фабрики. создает продукт
{
    virtual ~AbstractShopFactory() {}
    
    //создание книги, фильма
    virtual Book* createBook() const = 0;
    virtual Movie* createMovie() const = 0;
};



//фабрика выпускающая детские товары
struct ChildrenGoodsFactory : AbstractShopFactory
{
    Book* createBook() const override
    {
        return new FairyTale(); //делает сказку
    }
    Movie* createMovie() const override
    {
        return new Cartoon(); //делает мультфильм
    }
};

//фабрика делает учебные товары
struct EducationalGoodsFactory : AbstractShopFactory
{
    Book* createBook() const override {
        return new TextBook(); //делает учебник
    }
    Movie* createMovie() const override {
        return new DocumentaryMovie(); //делает документальный фильм
    }
};


void clientOrderProcess(AbstractShopFactory const & factory) {
    Book* myBook = factory.createBook();
    Movie* myMovie = factory.createMovie();

    // Просто вызываем действия объектов
    myBook->read();
    myMovie->watch();

    // Очищаем память
    delete myBook;
    delete myMovie;
}

int main() {
    cout << "Проверка работы фабрики" << endl << endl;

    cout << "Категория: Детские товары" <<endl;
    ChildrenGoodsFactory childrenFactory;
    clientOrderProcess(childrenFactory); 

    cout <<endl;
    cout << "Категория: Учебные товары" << endl;
    EducationalGoodsFactory eduFactory;
    clientOrderProcess(eduFactory); 

    return 0;
}
