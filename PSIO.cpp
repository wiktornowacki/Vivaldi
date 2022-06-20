#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <cctype>
#include "SimpleSerial.h"
#include "opencv2/core/utils/logger.hpp"
#include "Engine.h"
#include "Robot.h"
#include <vector>

char com_port[] = "\\\\.\\COM8";
DWORD COM_BAUD_RATE = CBR_9600;
const int INIT_X = 10;
const int INIT_Y = 280;
const int KAFELEK_X = 120;
const int KAFELEK_Y = KAFELEK_X;
const int min_area = 2000;
const int THRESHOLD = 40;

int ktory_O_pobrac = 0;
int ktory_X_pobrac = 0;

const std::pair<int, int> POZYCJE_O[5] = {
    std::make_pair(0,0),
    std::make_pair(95,0),
    std::make_pair(190,0),
    std::make_pair(285,0),
    std::make_pair(380,0)
};

const std::pair<int, int> POZYCJE_X[5] = {
    std::make_pair(0,90),
    std::make_pair(95,90),
    std::make_pair(190,90),
    std::make_pair(285,90),
    std::make_pair(380,90)
};

const std::pair<int, int> SZACHOWNICA[3][3] = {
    {std::make_pair(10,190),std::make_pair(110,190),std::make_pair(210,190)},
    {std::make_pair(10,290),std::make_pair(110,290),std::make_pair(210,290)},
    {std::make_pair(10,390),std::make_pair(110,390),std::make_pair(210,390)}
};

char board[3][3] =
{
    { '_', '_', '_' },
    { '_', '_', '_' },
    { '_', '_', '_' }
};

enum FIG {
    O = 1,
    X = 2,
    NONE = 0,
    ERR = -1
};

enum TRYB_PRACY {
    MANUAL = 0,
    KONSOLA = 1,
    MYSZ = 2
};

TRYB_PRACY mode = TRYB_PRACY::MYSZ;
/*********************************************/

FIG getContour(const cv::Mat& image_test, cv::Mat& image_draw) {
    std::vector<std::vector<cv::Point>> contours;
    std::vector < cv::Vec4i> hierarchy;
    cv::findContours(image_test, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> conPoly(contours.size());
    std::cout << "==========" << std::endl;
    
    int count_contours_OK = 0;
    FIG type;

    for (int i = 0; i < contours.size(); i++) {
        int area = cv::contourArea(contours[i]);
        std::cout << area << endl;

        if (area > min_area) {
            float peri = cv::arcLength(contours[i], true);
            cv::approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
            cv::drawContours(image_draw, conPoly, i, cv::Scalar(255, 0, 255), 2);
            std::cout << conPoly[i].size() << std::endl;
            if (conPoly[i].size() < 10) type = O;
            else type = X;
            count_contours_OK++;
        }
    }

    if (count_contours_OK == 1) {
        return type;
    }
    else if (count_contours_OK == 0) {
        return FIG::NONE; }
    else {
        return FIG::ERR;
    }
}

int main()
{   
    sf::Font font;
    sf::SoundBuffer full, kolko_dla_Ciebie, krzyzyk_dla_Ciebie, kolko_robot, krzyzyk_robot;
    std::cout << full.loadFromFile("full.wav") << kolko_dla_Ciebie.loadFromFile("kolko_dla_Ciebie.wav") << krzyzyk_dla_Ciebie.loadFromFile("krzyzyk_dla_Ciebie.wav") << kolko_robot.loadFromFile("kolko_robot.wav") << krzyzyk_robot.loadFromFile("krzyzyk_robot.wav") << std::endl;
    std::cout << font.loadFromFile("ariblk.ttf");
    sf::Sound sound;

    Robot robot(com_port, COM_BAUD_RATE);
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);
    cv::VideoCapture cap(1);

    sf::Texture X, O, NONE, GRACZ, ROBOT;
    std::cout << X.loadFromFile("x.png") << O.loadFromFile("o.png") << NONE.loadFromFile("none.png") <<
        GRACZ.loadFromFile("gracz.png") << ROBOT.loadFromFile("robot.png") << std::endl;

    char znak_dla_robota = 'o';
    bool czy_teraz_gra_robot = true;

    sf::Sprite wybor_gracza_kolko; wybor_gracza_kolko.setTexture(O); wybor_gracza_kolko.setPosition(10, 50); wybor_gracza_kolko.setScale(0.2, 0.2);
    sf::Sprite wybor_gracza_krzyzyk; wybor_gracza_krzyzyk.setTexture(X); wybor_gracza_krzyzyk.setPosition(300, 50); wybor_gracza_krzyzyk.setScale(0.2, 0.2);
    sf::Sprite kto_zaczyna_robot; kto_zaczyna_robot.setTexture(ROBOT); kto_zaczyna_robot.setPosition(10, 330); kto_zaczyna_robot.setScale(0.2, 0.2);
    sf::Sprite kto_zaczyna_gracz; kto_zaczyna_gracz.setTexture(GRACZ); kto_zaczyna_gracz.setPosition(300, 330); kto_zaczyna_gracz.setScale(0.2, 0.2);

    sf::RenderWindow window(sf::VideoMode(1000, 700), "PSIO 2022", sf::Style::Close);
    Engine engine;

    int MODE_WINDOW = 0;

    while (window.isOpen()) {
        window.clear(sf::Color::Black);
        if (MODE_WINDOW == 0) {

            std::vector <sf::Text> napisy(5);

            for (auto& el : napisy) {
                el.setFont(font);
                el.setFillColor(sf::Color::White);
                el.setCharacterSize(32);
            }

            napisy[0].setString(L"Proszę wybrać figurę, którą ma grać robot");
            napisy[1].setString(L"Proszę wybrać, kto zaczyna grę");
            napisy[2].setString(L"Aby koontynuować, proszę nacisnąć spację...");

            napisy[0].setPosition(10, 10);
            napisy[1].setPosition(10, 290);
            napisy[2].setPosition(10, 650);
            napisy[3].setPosition(10, 550);
            napisy[4].setPosition(10, 600);

            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();

                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
                        if (wybor_gracza_kolko.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
                            znak_dla_robota = 'o';
                            engine.SetZnak(znak_dla_robota);
                        }
                        else if (wybor_gracza_krzyzyk.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
                            znak_dla_robota = 'x';
                            engine.SetZnak(znak_dla_robota);
                        }
                        else if (kto_zaczyna_gracz.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
                            czy_teraz_gra_robot = false;
                        }
                        else if (kto_zaczyna_robot.getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
                            czy_teraz_gra_robot = true;
                        }
                    }
                }

                if (event.type == sf::Event::KeyReleased) {
                    if (event.key.code == sf::Keyboard::Space) {
                        MODE_WINDOW = 1;
                    }
                    else if (event.key.code == sf::Keyboard::Num1) {
                        mode = TRYB_PRACY::MYSZ;
                    }
                    else if (event.key.code == sf::Keyboard::Num2) {
                        mode = TRYB_PRACY::KONSOLA;
                    }
                    else if (event.key.code == sf::Keyboard::Num3) {
                        mode = TRYB_PRACY::MANUAL;
                    }
                }
            }

            string tekst_decyzji = "Aktualny wybór to [";
            tekst_decyzji.push_back((char)(std::toupper(znak_dla_robota)));
            tekst_decyzji+="], ponadto gre rozpocznie";
            if (czy_teraz_gra_robot) {
                tekst_decyzji += " Robot";
            }
            else {
                tekst_decyzji += "sz Ty";
            }
            napisy[3].setString(tekst_decyzji);
            
            string info_tryb_pracy = "Wybrany tryb pracy (1-2-3) to ";
            if (mode == TRYB_PRACY::MYSZ) info_tryb_pracy += "MYSZ (GUI)";
            else if (mode == TRYB_PRACY::KONSOLA) info_tryb_pracy += "KONSOLA";
            else info_tryb_pracy += "MANUAL";
            napisy[4].setString(info_tryb_pracy);

            for (const auto& el : napisy) {
                window.draw(el);
            }
            window.draw(wybor_gracza_kolko);
            window.draw(wybor_gracza_krzyzyk);
            window.draw(kto_zaczyna_robot);
            window.draw(kto_zaczyna_gracz);
        }
        else if (MODE_WINDOW == 1) {
            cv::Mat image;
            for (int i = 0; i < 3; i++) {
                std::this_thread::sleep_for(500ms);
                cap.read(image);
            }

            //Wymiary skalowania
            int w = 640;
            int h = 480;

            cv::resize(image, image, cv::Size(w, h), cv::INTER_LINEAR);
            cv::rotate(image, image, cv::ROTATE_90_CLOCKWISE);
            cv::Mat image_copy;
            image.copyTo(image_copy);

            std::vector<cv::Mat> kwadraty;

            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    cv::Rect roi;
                    roi.width = KAFELEK_X;
                    roi.height = KAFELEK_Y;
                    roi.x = INIT_X + i * KAFELEK_X;
                    roi.y = INIT_Y + j * KAFELEK_Y;

                    cv::Mat kwadrat = image(roi);
                    kwadraty.emplace_back(kwadrat);
                    cv::rectangle(image_copy, roi, cv::Scalar(0, 255, 0), 2);
                }
            }

            //cv::imshow("Input Image", image_copy);

            vector<sf::Image> images(kwadraty.size());
            vector<sf::Texture> textures(kwadraty.size());
            vector<sf::Sprite> sprites(kwadraty.size());
            vector<FIG> figures;
            vector<sf::Sprite> visualize(kwadraty.size());
            sf::Event event;

            for (auto& el : kwadraty) {
                cv::Mat imgGray, imgBlur, imgCanny, imgDil, imgErode;
                cv::cvtColor(el, imgGray, cv::COLOR_BGR2GRAY);
                cv::GaussianBlur(imgGray, imgBlur, cv::Size(7, 7), 3, 0);
                cv::Canny(imgBlur, imgCanny, THRESHOLD, 3 * THRESHOLD);
                cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
                cv::dilate(imgCanny, imgDil, kernel);
                FIG type = getContour(imgDil, el);
                figures.emplace_back(type);
                cv::cvtColor(el, el, cv::COLOR_BGR2RGBA);
            }

            for (int i = 0; i < images.size(); i++) {
                images[i].create(kwadraty[i].cols, kwadraty[i].rows, kwadraty[i].ptr());
                textures[i].loadFromImage(images[i]);
                sprites[i].setTexture(textures[i]);

                //std::cout << figures[i];
                char znak = '_';
                if (figures[i] == FIG::X) {
                    visualize[i].setTexture(X);
                    znak = 'x';
                }
                else if (figures[i] == FIG::O) {
                    visualize[i].setTexture(O);
                    znak = 'o';
                }
                else if (figures[i] == FIG::NONE) {
                    visualize[i].setTexture(NONE);
                    znak = '_';
                }
                else throw("FIGURE RECOGNITION ERROR");

                board[i % 3][i / 3] = znak;
            }

            for (auto& el : visualize) {
                el.setScale((float)KAFELEK_X / el.getGlobalBounds().width, (float)KAFELEK_Y / el.getGlobalBounds().height);
            }

            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int x = sprites[i * 3 + j].getGlobalBounds().width;
                    int y = sprites[i * 3 + j].getGlobalBounds().height;
                    sprites[i * 3 + j].setPosition((int)(20 + i * x * 1.2), (int)(20 + j * y * 1.2));
                    visualize[i * 3 + j].setPosition((int)(50 + 3 * 1.2 * x + i * x * 1.2), (int)(20 + j * y * 1.2));
                    window.draw(visualize[i * 3 + j]);
                    window.draw(sprites[i * 3 + j]);
                }
            }

            window.display();
            char ktos_wygral = engine.maybe_win(board);
            if (ktos_wygral != '_') {
                if (ktos_wygral != 'F') {
                    std::cout << std::endl << "Wygral " << (char)(std::toupper(ktos_wygral)) << " !!!!!" << std::endl;
                    if (ktos_wygral == 'x' && znak_dla_robota == 'x') sound.setBuffer(krzyzyk_robot);
                    else if (ktos_wygral == 'o' && znak_dla_robota == 'o') sound.setBuffer(kolko_robot);
                    else if (ktos_wygral == 'x' && znak_dla_robota == 'o') sound.setBuffer(krzyzyk_dla_Ciebie);
                    else if (ktos_wygral == 'o' && znak_dla_robota == 'x') sound.setBuffer(kolko_dla_Ciebie);
                }
                else {
                    std::cout << "Nikt nie wygral, skonczyly sie puste gniazda na szachownicy :(" << std::endl;
                    sound.setBuffer(full);
                }
                sound.play();
                while (sound.getStatus() == sf::Sound::Playing) {
                    std::cout << "Audio..." << std::endl;
                }
                break;
            }
            Engine::Move bestMove = engine.findBestMove(board);

            if (czy_teraz_gra_robot) {
                cout << "Teraz gra robot!" << endl;
                if (znak_dla_robota == 'o') {
                    robot.Move(POZYCJE_O[ktory_O_pobrac]);
                    ktory_O_pobrac++;
                }
                else {
                    robot.Move(POZYCJE_X[ktory_X_pobrac]);
                    ktory_X_pobrac++;
                }

                robot.Pick();
                robot.Move(SZACHOWNICA[bestMove.row][bestMove.col]);
                robot.Place();
                robot.Move(0, 0);

                //cv::waitKey(0);
            }

            else {
                int row, col;
                    
                if (mode == TRYB_PRACY::MANUAL) {
                    std::cout << "Poloz karteczke i nacisnij dowolny klawisz, aby koontynuowac...";
                    char c;
                    cin >> c;
                }
                else if (mode == TRYB_PRACY::KONSOLA) {
                    cout << "Podaj nr wiersza (0-2) "; cin >> row;
                    cout << "Podaj nr kolumny (0-2) "; cin >> col;
                }
                else {
                    bool uzytkownik_wybral = false;
                    while (!uzytkownik_wybral) {
                        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                            std::cout << "Middle mouse button is pressed" << std::endl;
                            sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
                            for (int i = 0; i < visualize.size(); i++) {
                                if (visualize[i].getGlobalBounds().contains(mouse_pos.x, mouse_pos.y)) {
                                    if (visualize[i].getTexture() == &NONE) {
                                        row = i % 3;
                                        col = i / 3;
                                        uzytkownik_wybral = true;
                                        break;
                                    }
                                }
                            }
                        }
                        std::cout << "Czekam..." << std::endl;
                    }
                }

                if (mode != TRYB_PRACY::MANUAL) {
                    if (znak_dla_robota == 'x') {
                        robot.Move(POZYCJE_O[ktory_O_pobrac]);
                        ktory_O_pobrac++;
                    }
                    else {
                        robot.Move(POZYCJE_X[ktory_X_pobrac]);
                        ktory_X_pobrac++;
                    }

                    robot.Pick();
                    robot.Move(SZACHOWNICA[row][col]);
                    robot.Place();
                    robot.Move(0, 0);
                }
                //cv::waitKey(0);
            }

            cv::destroyAllWindows();
            czy_teraz_gra_robot = !czy_teraz_gra_robot;

        }
        window.display();
    }
    return 0;
}