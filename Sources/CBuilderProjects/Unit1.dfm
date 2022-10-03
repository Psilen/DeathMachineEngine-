object Form1: TForm1
  Left = 192
  Top = 265
  Width = 1088
  Height = 471
  Caption = 'Form1'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  WindowState = wsMaximized
  OnCloseQuery = FormCloseQuery
  OnKeyDown = FormKeyDown
  OnKeyUp = FormKeyUp
  OnMouseDown = FormMouseDown
  OnMouseUp = FormMouseUp
  PixelsPerInch = 96
  TextHeight = 13
  object Render: TTimer
    OnTimer = RenderTimer
    Left = 528
    Top = 224
  end
  object Timer1: TTimer
    OnTimer = Timer1Timer
    Left = 560
    Top = 224
  end
end
