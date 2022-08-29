#include <ncurses.h>
#include <string>

// Пока оно true - будем крутиться в игре,
// если станет false - выходим.
static bool run = false;

// Если true - значит проиграли и надо нарисовать плашку Game Over
static bool gameOver;

// Если true - у нас пауза, ничего не происходит, ждем нажатия кнопок
static bool pause;

//Ширина и высота терминального окна
static int termWidth;
static int termHeight;

// Координаты головы
static int playerX;
static int playerY;

// Координаты еды
static int fruitX;
static int fruitY;

// Смещения по осям - куда бежать
// 1 или -1 - бежим вдоль оси. 0 - не бежим
static int dx;
static int dy;

// Очки
static int score;

// Начало и конец питона в массиве
static int pythonStart;
static int pythonEnd;

// Максимальный размер массива, чтобы можно было теоретически покрыть весь экран
static int maxPythonSize;

// Предварительное описание, реализация позже будет.
void DropFruit();

// Структурка для хранения координат в массиве.
struct coord
{
    int x;
    int y;
};

// Массив для координат тела - определение
static coord * python;

// Тут мы инициализируем все что надо
void InitGame()
{
    // Инициализируем ncurses и выставляем ей разное
    initscr();
    raw();
    cbreak();
    noecho();
    keypad(stdscr, true);

    // Таймаут ввода, он же скорость игры
    timeout(200);

    // Получаем размер терминального окна
    getmaxyx(stdscr, termHeight, termWidth);

    // Убираем курсор
    curs_set(0);

    //Рисуем рамку
    box(stdscr, 0, 0);

    //Инициализируем разные переменные
    run = true;
    gameOver = false;
    pause = false;

    // Голову в центр экрана
    playerX = termWidth / 2;
    playerY = termHeight / 2;

    // Едем вправо
    dx = 1;
    dy = 0;

    // Очков пока ноль
    score = 0;

    // Высчитываем максимальный размер питона, исходя из размера экрана
    // (ну вдруг кто-то умудрится)
    maxPythonSize = (termWidth - 2) * (termHeight - 2);

    // Инициализируем массив под координаты частей тела
    python = new coord[maxPythonSize]();

    // Задаем начальные координаты
    python[0].x = playerX;
    python[0].y = playerY;

    // и указатели на хвост и голову
    pythonStart = 0;
    pythonEnd = 0;

    // Бросаем первую еду
    DropFruit();
}


// Тут завершается ncurses
void EndGame()
{
    endwin();
}

// Процедура, ловящая ввод и обрабатывающая его
void Input()
{
    // Получаем нажатую клавишу
    int c = getch();

    // в зависимости от нажатой клавиши правим смещения
    switch (c) {

    // Если нажали q - ставим run в false, потому как надо выходить
    case 'q':
        run = false;
        break;
    case ' ':
        pause = !pause;
        break;
    case KEY_UP:
        // На каждый чих надо проверить, не пытается ли игрок поехать в противоположную сторону.
        // Если так - нафиг. Ничего не делаем.
        if (dy == 1)
            break;
        dx = 0;
        dy = -1;
        break;
    case KEY_DOWN:
        if (dy == -1)
            break;
        dx = 0;
        dy = 1;
        break;
    case KEY_LEFT:
        if (dx == 1)
            break;
        dx = -1;
        dy = 0;
        break;
    case KEY_RIGHT:
        if (dx == -1)
            break;
        dx = 1;
        dy = 0;
    }
}

// Микроскопическая процедурка, печатающая очки в углу
void PrintScore()
{
    mvprintw(0, termWidth - 20, "Score: %5d", score);
}


// Тут мы двигаем голову и проверяем все что можно
void Logic()
{
    // Двигаем по X, просто прибавляя к координатам смещение
    playerX += dx;

    // Проверяем на выход за рамку
    if (playerX > termWidth - 2 || playerX < 1)
    {
        // Если вылезли - ставим флаг GameOver и run в false, чтобы выйти
        gameOver = true;
        run = false;
        return;
    }

    playerY += dy;
    if (playerY > termHeight - 2 || playerY < 1)
    {
        gameOver = true;
        run = false;
        return;
    }

    // Тут мы проходим по куску массива с телом между началом и концом
    // чтобы выяснить, не куснули ли сами себя.
    int i = pythonStart;
    while (i <= pythonEnd)
    {
        // Если координаты головы совпадают с координатами из массива - Game over
        if (playerX == python[i].x && playerY == python[i].y)
        {
            gameOver = true;
            run = false;
            return;
        }
        i++;
    }

    // Проверяем, не куснули ли мы еду
    if (playerX == fruitX && playerY == fruitY)
    {
        // Если куснули - выкидываем новую
        DropFruit();

        // Добавляем очков
        score++;

        // Сдвигаем хвост
        pythonEnd++;

        // Если хвост дошел до конца массива - переставляем его на начало
        if (pythonEnd == maxPythonSize)
            pythonEnd = 0;

        // Выводим новые очки в угол
        PrintScore();
    }

    // Двигаем питона

    // Стираем хвост
    mvprintw(python[pythonEnd].y, python[pythonEnd].x, " ");

    // Двигаем указатель головы назад по массиву
    pythonStart--;

    // Не забывая его переставить, если он дошел до нуля
    if (pythonStart < 0)
        pythonStart = maxPythonSize - 1;

    // Тоже и с указателем хвоста
    pythonEnd--;
    if (pythonEnd < 0)
        pythonEnd = maxPythonSize - 1;

    // Записываем новое положение головы по указателю
    python[pythonStart].x = playerX;
    python[pythonStart].y = playerY;

    // Рисуем по новым координатам голову
    mvprintw(playerY, playerX, "@");
}

// Жуткая процедура рисования окошка с Game Over
void DrawGameOver()
{
    int winWidth = termWidth / 2 + 10;
    int winHeight = 7;
    int winX = (termWidth - winWidth) / 2;
    int winY = (termHeight - winHeight) / 2;
    WINDOW * gameOverWin = newwin(winHeight, winWidth, winY, winX);
    box(gameOverWin, 0, 0);
    std::string str1 = "GAME OVER!";
    std::string str2 = "Score: " + std::to_string(score);
    mvwprintw(gameOverWin, 2, (winWidth - (int)str1.length()) / 2, str1.c_str());
    mvwprintw(gameOverWin, 4, (winWidth - (int)str2.length()) / 2, str2.c_str());
    wrefresh(gameOverWin);
    notimeout(gameOverWin, TRUE);
    wgetch(gameOverWin);
}

// Здесь мы крутимся, пока игра идет
void GameLoop()
{
    while (run)
    {
        // Обрабатываем ввод
        Input();

        if (!pause)
        {
          // Обрабатываем логику
          Logic();
        }
    }

    // Сюда мы выйдем, если игра так или иначе кончится
    // Если Game over - рисуем плашку с очками
    if (gameOver)
        DrawGameOver();
}

// Выбрасываем новую еду
void DropFruit()
{
	// Делаем флаг про то, что координаты не пересекаются с питоном
	bool goodCoords = true;

	// Крутимся до тех пор, пока флаг не будет true
	// (Он сразу true, но если координаты попадут на змею, он станет false
	// и все уйдет на второй круг. А если не попадут - не уйдет.)
	do
	{
		// Новые координаты в переменные
		fruitX = (rand() % (termWidth - 2)) + 1;
		fruitY = (rand() % (termHeight - 2)) + 1;

		// Тут мы проходим по куску массива с телом между началом и концом
		// чтобы выяснить, не попадает ли новая еда поперек змеи.
		for (int i = 0; i < pythonEnd; i++)
		{
			if (fruitX == python[i].x && fruitY == python[i].y)
			{
				// Если попали - сбрасываем флаг и выходим из цикла.
				goodCoords = false;
				break;
			}
		}
	} while (!goodCoords);
	
	// и нарисовать
    mvprintw(fruitY, fruitX, "#");
}

// Главная функция - просто вызывает все предыдущие последовательно
int main(int argc, const char * argv[]) {
    InitGame();

    getch();

    PrintScore();

    GameLoop();

    EndGame();
    return 0;
}
