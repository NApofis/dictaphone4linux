#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref

#include "ui.h"

using namespace ftxui;


int main()
{
    UIConfigHandler config;
    UIDeleterRecords deleter(config.path_for_records);

    auto devices = portaudio_devices::list_input_devices();
    std::vector<std::string> i_devices_4_show = {"None"};
    int index = 0;
    config.input_devices.push_back({index, {0, "None", "None", false}});
    for (const auto& d : devices)
    {
        i_devices_4_show.push_back(d.description);
        index++;
        config.input_devices.emplace_back(index, d);
        if(d.name == config.selected_input_device_name())
        {
            config.selected_input_device = index;
        }
    }

    devices = portaudio_devices::list_output_devices();
    std::vector<std::string> o_devices_4_show = {"None"};
    index = 0;
    config.output_devices.push_back({0, {0, "None", "None", false}});
    for (const auto& d : devices)
    {
        o_devices_4_show.push_back(d.description);
        index++;
        config.output_devices.emplace_back(index, d);
        if(d.name == config.selected_output_device_name())
        {
            config.selected_output_device = index;
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
        Container::Horizontal({cancel_button})
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
                hbox(text(" Размер одной записи в минутах : "), element_file_size->Render()),
                hbox(text(" Время хранения записей в минутах : "), element_shelf_life->Render()),
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

            hbox(filler(), cancel_button->Render()) | align_right,
        }) | border;

    });

    screen.Loop(renderer);
    return 0;
}
