
#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>

#include <thread>

#include "StringUtils.h"


std::vector<std::vector<std::string>> devices = std::vector<std::vector<std::string>>();



std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

std::vector<HotKey> hotKeys = std::vector<HotKey>();

bool active = true;

bool requireRefresh = false;

std::string lastID = "";

void runHotkeys()
{
	while (active)
	{
		for (int i = hotKeys.size() - 1; i >= 0; i--)
		{
			if (GetAsyncKeyState(hotKeys.at(i).keyCode))
			{
				if (true||hotKeys.at(i).active)
				{
					// Set output audio device by ID
					std::string id = hotKeys.at(i).id;
					lastID = id;
					try {
						exec((std::string("AudioInerface.exe -s ") + id).c_str());
					}
					catch (std::exception e)
					{

					}
					requireRefresh = true;
					Beep(800, 200);
				}
			}
			_sleep(1);
		}
		_sleep(30);
	}
}

std::thread myThread(runHotkeys);

namespace CppCLRWinFormsProject {
	std::string currId = "";

	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:

		void updateAfterChange()
		{
			updateListView();
		}

		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			active = false;
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ListBox^ listBox1;
	private: System::Windows::Forms::Label^ label1;
	private: System::Windows::Forms::TextBox^ textBox1;
	private: System::Windows::Forms::Label^ label2;
	private: System::Windows::Forms::Label^ label3;
	private: System::Windows::Forms::TextBox^ textBox2;
	private: System::Windows::Forms::Button^ button1;
	private: System::Windows::Forms::Label^ label4;
	private: System::Windows::Forms::Label^ label5;
	private: System::Windows::Forms::CheckedListBox^ checkedListBox1;
	protected:


		virtual void OnPaint(System::Windows::Forms::PaintEventArgs^ e) override
		{
			// Call the base class implementation
			Form::OnPaint(e);

			if (requireRefresh)
			{
				requireRefresh = false;
				label4->Text = gcnew String(lastID.c_str());
				updateListView();
			}

		}




	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

		

		std::string to_string(String^ str)
		{
			std::string stri = "";
			for (int i = 0; i < str->Length; ++i)
			{
				stri.push_back(static_cast<char>(str[i]));
			}
			return stri;
		}

		void listBox1_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e)
		{
			if (listBox1->SelectedItem != nullptr)
			{
				int index = listBox1->SelectedIndex-1;
				if (index < devices.size())
				{
					std::string name = devices.at(index).at(1);
					currId = devices.at(index).at(2);
					label5->Text = gcnew String(currId.c_str());
					textBox1->Text = gcnew String(name.c_str());
				}
			}
		}

		void textBox2_KeyDown(System::Object^ sender, System::Windows::Forms::KeyEventArgs^ e)
		{
			textBox2->Text = gcnew String(std::to_string(e->KeyValue).c_str());
		}


		void button1_MouseClick(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
		{
			// Check if the left mouse button was clicked
			if (e->Button == System::Windows::Forms::MouseButtons::Left)
			{
				std::string str = to_string(textBox2->Text);
				if (!str.empty())
				{
					HotKey hk = HotKey();
					hk.active = true;
					hk.keyCode = std::stoi(str);

					hk.id = std::string(currId);

					hotKeys.push_back(hk);

					textBox2->Text = gcnew String("");
					updateHKview();
				}
			}
		}

		void checkedListBox1_ItemCheck(System::Object^ sender, System::Windows::Forms::ItemCheckEventArgs^ e)
		{
			int index = e->Index;
			hotKeys.at(index).active = !hotKeys.at(index).active;
			void updateHKview();
		}


		void updateHKview()
		{
			checkedListBox1->Items->Clear();
			for (int i = 0; i < hotKeys.size(); i++)
			{
				checkedListBox1->Items->Add(gcnew String(("Hot key: "+std::to_string(hotKeys.at(i).keyCode)).c_str()));
				checkedListBox1->SetItemChecked(i, hotKeys.at(i).active);
			}
		}

		void updateWindow(System::Object^ sender, System::Windows::Forms::ItemCheckEventArgs^ e)
		{
			int index = e->Index;
			hotKeys.at(index).active = !hotKeys.at(index).active;
			void updateHKview();
		}


		void initListView()
		{
			listBox1->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::listBox1_SelectedIndexChanged);

			textBox2->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::textBox2_KeyDown);
			textBox2->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &Form1::textBox2_KeyDown);

			button1->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::button1_MouseClick);

			checkedListBox1->ItemCheck += gcnew System::Windows::Forms::ItemCheckEventHandler(this, &Form1::checkedListBox1_ItemCheck);

		}

		void getDevices()
		{
			devices.clear();
			std::string csv = exec("AudioInerface.exe -l");

			std::vector<std::string> lines = StringUtils::split(csv, "\n");

			for (int i = 0; i < lines.size(); i++)
			{
				std::vector<std::string> values = StringUtils::split(lines.at(i), ";");
				devices.push_back(values);
			}
		
		}

		std::string spaces(int num)
		{
			std::string result = std::string();
			for (int c = 0; c < num; c++)
			{
				result.append(" ");
			}
			return result;
		}

		std::string displayLine(std::vector<std::string> vars)
		{
			std::string result = std::string();
			std::string name = vars.at(1);
			std::string active = vars.at(0);
			result.append(active);
			result.append(spaces(50 - result.length()));
			result.append(name);
			result.append(spaces(150-result.length()));
			return result;
		}

		void updateListView()
		{
			try
			{
				getDevices();
			}
			catch (std::exception e)
			{

			}


			listBox1->Items->Clear();
			std::vector<std::string> head = std::vector<std::string>();
			head.push_back("Active");
			head.push_back("Name");
			
			listBox1->Items->Add(gcnew String(displayLine(head).c_str()));

			for (int i = 0; i < devices.size(); i++)
			{
				std::vector<std::string> infos = devices.at(i);

				if (infos.size() > 2)
				{
					listBox1->Items->Add(gcnew String(displayLine(infos).c_str()));
				}
			}

		}

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->listBox1 = (gcnew System::Windows::Forms::ListBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->textBox2 = (gcnew System::Windows::Forms::TextBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->checkedListBox1 = (gcnew System::Windows::Forms::CheckedListBox());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// listBox1
			// 
			this->listBox1->FormattingEnabled = true;
			this->listBox1->ItemHeight = 16;
			this->listBox1->Location = System::Drawing::Point(75, 53);
			this->listBox1->Name = L"listBox1";
			this->listBox1->Size = System::Drawing::Size(700, 548);
			this->listBox1->TabIndex = 0;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(790, 140);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(53, 16);
			this->label1->TabIndex = 1;
			this->label1->Text = L"Device:";
			// 
			// textBox1
			// 
			this->textBox1->Location = System::Drawing::Point(849, 137);
			this->textBox1->Name = L"textBox1";
			this->textBox1->ReadOnly = true;
			this->textBox1->Size = System::Drawing::Size(293, 22);
			this->textBox1->TabIndex = 2;
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Font = (gcnew System::Drawing::Font(L"Arial Rounded MT Bold", 16.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label2->Location = System::Drawing::Point(911, 53);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(137, 32);
			this->label2->TabIndex = 3;
			this->label2->Text = L"Hot Keys";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(790, 182);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(53, 16);
			this->label3->TabIndex = 4;
			this->label3->Text = L"Hotkey:";
			// 
			// textBox2
			// 
			this->textBox2->Location = System::Drawing::Point(849, 182);
			this->textBox2->Name = L"textBox2";
			this->textBox2->Size = System::Drawing::Size(293, 22);
			this->textBox2->TabIndex = 5;
			// 
			// button1
			// 
			this->button1->Font = (gcnew System::Drawing::Font(L"Arial Rounded MT Bold", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button1->Location = System::Drawing::Point(793, 224);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(349, 40);
			this->button1->TabIndex = 6;
			this->button1->Text = L"Add Hotkey";
			this->button1->UseVisualStyleBackColor = true;
			// 
			// checkedListBox1
			// 
			this->checkedListBox1->FormattingEnabled = true;
			this->checkedListBox1->Location = System::Drawing::Point(793, 302);
			this->checkedListBox1->Name = L"checkedListBox1";
			this->checkedListBox1->Size = System::Drawing::Size(349, 310);
			this->checkedListBox1->TabIndex = 7;
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(799, 622);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(44, 16);
			this->label4->TabIndex = 8;
			this->label4->Text = L"label4";
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(350, 622);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(44, 16);
			this->label5->TabIndex = 9;
			this->label5->Text = L"label5";
			initListView();
			updateListView();
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1200, 647);
			this->Controls->Add(this->label5);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->checkedListBox1);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->textBox2);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->listBox1);
			this->Name = L"Form1";
			this->Text = L"AudioSwitch";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
	};
}
