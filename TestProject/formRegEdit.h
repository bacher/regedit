//---------------------------------------------------------------------------

#ifndef formRegEditH
#define formRegEditH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TfrmRegEdit : public TForm
{
__published:	// IDE-managed Components
    TButton *btn1;
    TButton *btn2;
    TButton *btn3;
    TButton *btn4;
    TButton *btn5;
    TButton *btn6;
    void __fastcall btn1Click(TObject *Sender);
    void __fastcall btn2Click(TObject *Sender);
    void __fastcall btn3Click(TObject *Sender);
    void __fastcall btn4Click(TObject *Sender);
    void __fastcall btn5Click(TObject *Sender);
    void __fastcall btn6Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall TfrmRegEdit(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmRegEdit *frmRegEdit;
//---------------------------------------------------------------------------
#endif
