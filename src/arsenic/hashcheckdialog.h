﻿#ifndef HASHCHECKDIALOG_H
#define HASHCHECKDIALOG_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class HashCheckDialog;
}

class HashCheckDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HashCheckDialog(QWidget *parent = nullptr);
    ~HashCheckDialog() Q_DECL_OVERRIDE;

private:
    Ui::HashCheckDialog *ui;
    bool cancel_calculation;
    bool isCalculating;

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;


private slots:
    void openFile();
    void calculate(const QString &text);
    void cancel();
    void messageBox(QMessageBox::Icon icon, const QString& title, const QString& message);
    void textChanged(const QString &text);
    void copyToClipboard();

};

#endif // HASHCHECKDIALOG_H