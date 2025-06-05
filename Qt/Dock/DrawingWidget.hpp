#pragma once

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

class DrawingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DrawingWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawGrid(QPainter &painter);

    static constexpr int GRID_SIZE = 20; // Taille de la grille en pixels
};