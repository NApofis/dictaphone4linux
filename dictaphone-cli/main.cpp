#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref

#include "ui.h"
#include "daemon_command.h"
#include "pulseaudio_device.h"

using namespace ftxui;


int main()
{
    UIConfigHandler config;
    UIDeleterRecords deleter(config.path_for_records);

    auto devices = pulseaudio::list_input_devices();
    std::vector<std::string> i_devices_4_show = {NONE_DEVICE};
    int index = 0;
    config.input_devices.emplace_back(index, pulseaudio::DeviceInfo{0, NONE_DEVICE, NONE_DEVICE});
    for (const auto& d : devices)
    {
        if(d.real)
        {
            i_devices_4_show.push_back(d.human_name);
            index++;
            config.input_devices.emplace_back(index, d);
            if(d.device == config.selected_input_device_name())
            {
                config.selected_input_device = index;
            }
        }
    }

    devices = pulseaudio::list_output_devices();
    std::vector<std::string> o_devices_4_show = {NONE_DEVICE};
    index = 0;
    config.output_devices.emplace_back(0, pulseaudio::DeviceInfo{0, NONE_DEVICE, NONE_DEVICE});
    for (const auto& d : devices)
    {
        if(d.real)
        {
            o_devices_4_show.push_back(d.human_name);
            index++;
            config.output_devices.emplace_back(index, d);
            if(d.device == config.selected_output_device_name())
            {
                config.selected_output_device = index;
            }
        }
    }

    auto element_input_device = Dropdown(i_devices_4_show, &config.selected_input_device);
    auto element_output_device = Dropdown(o_devices_4_show, &config.selected_output_device);

    auto element_path4records = Input(config.path_for_records.get(), "");
    auto element_file_size = Input(&config.file_minute_size, "");
    element_file_size |= CatchEvent([&](const Event& event) {
      return event.is_character() && !std::isdigit(event.character()[0]);
    });

    auto element_shelf_life = Input(&config.shelf_life, "");
    element_shelf_life |= CatchEvent([&](const Event& event) {
      return event.is_character() && !std::isdigit(event.character()[0]);
    });

    auto screen = ScreenInteractive::FitComponent();
    std::string message_config;

    auto save_button = Button("Сохранить", [&config, &screen, &message_config]()
    {
        config.save_config(message_config);
    }, ButtonOption::Animated(Color::Green));

    std::string message_deleter;
    auto element_delete_period_value = Input(&deleter.selected_delete_value, "");
    element_delete_period_value |= CatchEvent([&](const Event& event) {
      return event.is_character() && !std::isdigit(event.character()[0]);
    });
    auto element_delete_period = Dropdown(deleter.period, &deleter.selected_delete_period);
    auto delete_button = Button("Удалить", [&deleter, &message_deleter]
    {
        deleter.delete_records(message_deleter);
    }, ButtonOption::Animated(Color::Red));

    bool daemon_work = daemon_command::is_work();
    std::string record_button_current_text = daemon_work ? "Остановить запись" : "Начать запись";
    auto record_button = Button(&record_button_current_text, [& daemon_work, &record_button_current_text]()
    {
        if(daemon_work)
        {
            daemon_command::stop();
            daemon_work = false;
            record_button_current_text = "Начать запись";
        }
        else
        {
            daemon_command::start();
            daemon_work = true;
            record_button_current_text = "Остановить запись";
        }
    }, ButtonOption::Animated(Color::Default, Color::GrayDark, Color::Default, Color::White));

    auto cancel_button = Button("Закрыть", screen.ExitLoopClosure(), ButtonOption::Animated());


    auto components = Container::Vertical({
        Container::Horizontal({
            element_input_device, element_output_device
        }),
        Container::Horizontal({element_path4records}),
        Container::Horizontal({element_file_size}),
        Container::Horizontal({element_shelf_life}),
        Container::Horizontal({save_button}),
        Container::Horizontal({element_delete_period_value, element_delete_period, delete_button}),
        Container::Horizontal({record_button, cancel_button})
    });


    auto renderer = Renderer(components, [&] {
        return vbox({
            vbox({
                hbox(text("Конфигурация")),

                hbox({
                    text(" Устройство ввода : ") | vcenter,
                    element_input_device->Render(),
                    text(" Устройство вывода  : ") | vcenter,
                    element_output_device->Render()
                }),

                hbox(text(" Путь до папки для хранения записей : "), element_path4records->Render()),
                hbox(text(" Ориентировочный размер одной записи в минутах : "), element_file_size->Render()),
                hbox(text(" Время хранения записей в днях : "), element_shelf_life->Render()),
                hbox(text("")),
                hbox(save_button->Render()) | align_right,
                hbox(text(message_config)),
            }) | border,

            vbox({
                hbox(text("Удаление записей")),
                hbox({
                    text(" Период : ") | vcenter,
                    element_delete_period->Render()
                }),
                hbox({
                    text(" Количество : ") | vcenter,
                    element_delete_period_value->Render(),
                }),
                hbox(delete_button->Render() | align_right | flex),
                hbox(text(message_deleter))
            })| border,

            hbox(record_button->Render(), filler(), cancel_button->Render()) ,
        }) | border;

    });

    screen.Loop(renderer);
    return 0;
}
