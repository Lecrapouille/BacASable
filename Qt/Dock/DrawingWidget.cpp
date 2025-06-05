#include "DrawingWidget.hpp"
#include <QPainter>
#include <QPen>

DrawingWidget::DrawingWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setStyleSheet("background-color: white;");
}

void DrawingWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Dessiner la grille
    drawGrid(painter);

    QWidget::paintEvent(event);
}

void DrawingWidget::drawGrid(QPainter &painter)
{
    QPen gridPen(Qt::lightGray);
    gridPen.setWidth(1);
    painter.setPen(gridPen);

    int width = this->width();
    int height = this->height();

    // Dessiner les lignes verticales
    for (int x = 0; x <= width; x += GRID_SIZE) {
        painter.drawLine(x, 0, x, height);
    }

    // Dessiner les lignes horizontales
    for (int y = 0; y <= height; y += GRID_SIZE) {
        painter.drawLine(0, y, width, y);
    }

    // Dessiner les axes principaux en gras
    QPen axisPen(Qt::gray);
    axisPen.setWidth(2);
    painter.setPen(axisPen);

    // Axe vertical central
    int centerX = width / 2;
    centerX = (centerX / GRID_SIZE) * GRID_SIZE; // Aligner sur la grille
    painter.drawLine(centerX, 0, centerX, height);

    // Axe horizontal central
    int centerY = height / 2;
    centerY = (centerY / GRID_SIZE) * GRID_SIZE; // Aligner sur la grille
    painter.drawLine(0, centerY, width, centerY);
}