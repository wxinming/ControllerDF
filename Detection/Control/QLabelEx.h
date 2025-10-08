#pragma once
#pragma execution_character_set("utf-8")

#include <QLabel>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QApplication>
#include <QVector>
#include <libcan/libcan.h>
#include "../Config.h"

enum LabelMode {
	LM_NONE,
	LM_PAINT,
	LM_TOUCH
};

struct DETECTION_DLL_EXPORT TouchScreenInfo {
	int screenWidth;
	int screenHeight;
	int msgId;
	int msgSendPeriod;
	int msgSendCount;
	int touchStart;
	int touchLength;
	int touchPress;
	int touchRelease;
	int touchSlither;
	int xStart;
	int xLength;
	int yStart;
	int yLength;
	can::Matrix* canMatrix;
	can::Base** canBase;
};

namespace Dt
{
	class DETECTION_DLL_EXPORT QLabelEx : public QLabel
	{
		Q_OBJECT
	public:
		QLabelEx(QWidget* parent = nullptr);
		~QLabelEx();
		void setLabelMode(LabelMode mode);
		void getCoordinate(QVector<QPoint>* start, QVector<QPoint>* end);
		void clearCoordinate();
		void setTouchScreenInfo(const TouchScreenInfo& touch);
		void setShowCoordSizeLabel(QLabel* label);

	protected:
		void paintEvent(QPaintEvent* paintEvent) override;
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

	signals:
		void coordinateSignal(const QPoint& point);

	private:
		LabelMode m_mode = LabelMode::LM_NONE;
		bool m_press = false;
		int m_startX = 0;
		int m_startY = 0;
		int m_endX = 0;
		int m_endY = 0;
		QVector<QPoint> m_startCoord;
		QVector<QPoint> m_endCoord;
		TouchScreenInfo m_touch = { 0 };
		//主界面图像控件宽度
		int m_width = 720;
		//主界面图像控件高度
		int m_height = 480;
		QLabel* m_label = nullptr;
	};
}
